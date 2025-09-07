#include "ParameterServer.h"

#include <rcp_parameter.h>
#include <rcp_typedefinition.h>

#include <m_pd.h>

#include "PdMaxUtils.h"
#include "Optional.h"
#include "PdServerTransporter.h"
#include "Threading.h"
#include "rabbit.server.h"
#include "WebsocketServerTransporter.h"

using namespace PdMaxUtils;

static void parameterValueUpdatedCb(rcp_value_parameter* parameter, void* user)
{
    if (user)
    {
        rcp::ParameterServer* x = static_cast<rcp::ParameterServer*>(user);
        x->parameterUpdate(RCP_PARAMETER(parameter));
    }
}
static void bangCb(rcp_bang_parameter* param, void* user)
{
    if (user)
    {
        rcp::ParameterServer* x = static_cast<rcp::ParameterServer*>(user);
        x->parameterUpdate(RCP_PARAMETER(param));
    }
}

// synchronized from threaded transporters

static void pd_id_parameter_output(t_pd *obj, void *data)
{
    std::vector<t_atom>* _list = (std::vector<t_atom>*)data;

    if (obj != NULL)
    {
        t_rabbit_server_pd* x = (t_rabbit_server_pd*)obj;

        if (_list &&
            _list->size() > 1)
        {
            outlet_float(x->parameter_id_out, (*_list)[0].a_w.w_float);
            outlet_list(x->parameter_out, NULL, _list->size()-1, _list->data()+1);
        }
    }

    if (_list)
    {
        delete _list;
    }
}


namespace rcp
{

ParameterServer::ParameterServer(t_rabbit_server_pd* x, int argc, t_atom *argv)
    : ParameterServerClientBase(x)
    , m_x(x)
{

    // init pd struct
    m_x->clients = 0;

    m_x->parameter_out = outlet_new(&m_x->x_obj, &s_list);
    m_x->parameter_id_out = outlet_new(&m_x->x_obj, &s_float);
    m_x->client_count_out = outlet_new(&m_x->x_obj, &s_float);
    m_x->info_out = outlet_new(&m_x->x_obj, &s_list);

    setOutlets(m_x->parameter_out,
               m_x->parameter_id_out,
               m_x->info_out);


    // create server
    m_server = rcp_server_create(NULL);

    if (m_server == nullptr)
    {
        throw std::runtime_error("could not create rcp server");
    }

    m_manager = rcp_server_get_manager(m_server);

    // set application id
    rcp_server_set_id(m_server, "pd rcp server");


    std::string rhl_uri;

    // check arguments
    for (int i = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_SYMBOL)
        {
            if (strcmp(argv[i].a_w.w_symbol->s_name, "@rabbithole") == 0 &&
                i < argc-1)
            {
                i++;
                if (argv[i].a_type == A_SYMBOL)
                {
                    rhl_uri = std::string(argv[i].a_w.w_symbol->s_name);
                }
                else
                {
                    pd_error(m_x, "invalid rabbithole url");
                }
            }
            else if (strcmp(argv[i].a_w.w_symbol->s_name, "-raw") == 0)
            {
                m_raw = true;
            }

            // other arguments?
        }
    }

    if (m_raw)
    {
        // raw server
        m_x->raw_in = inlet_new(&m_x->x_obj, &m_x->x_obj.ob_pd, &s_list, gensym("__raw_input"));
        m_x->raw_out = outlet_new(&m_x->x_obj, &s_list);

        setRawOutlet(m_x->raw_out);

        m_transporter = new PdServerTransporter(this);
    }
    else
    {
        m_transporter = new WebsocketServerTransporter((t_pd*)x);
    }

    if (!m_transporter)
    {
        throw std::runtime_error("could not create rcp server transporter");
    }

    // add transporter
    rcp_server_add_transporter(m_server, m_transporter->transporter());


    if (!rhl_uri.empty())
    {
        setRabbithole(rhl_uri);
    }
}

ParameterServer::~ParameterServer()
{
    if (m_rabbitholeTransporter)
    {
        m_rabbitholeTransporter.reset();
    }

    rcp_server_free(m_server);
    m_server = nullptr;

    if (m_transporter)
    {
        m_transporter->unbind();
        delete m_transporter;
        m_transporter = nullptr;
    }


    // cleanup pd struct

    outlet_free(m_x->parameter_out);
    outlet_free(m_x->parameter_id_out);
    outlet_free(m_x->client_count_out);
    outlet_free(m_x->info_out);

    if (m_x->raw_out)
    {
        outlet_free(m_x->raw_out);
    }

    if (m_x->raw_in)
    {
        inlet_free(m_x->raw_in);
    }
}

// NOTE: called from transporter thread
void ParameterServer::outputIdParameterList(std::vector<t_atom>* list)
{
    pd_queue_mess(&pd_maininstance, (t_pd*)m_x, list, pd_id_parameter_output);
}

// port
int ParameterServer::port() const
{
    if (m_transporter)
    {
        return m_transporter->port();
    }

    return 0;
}

void ParameterServer::listen(int port)
{
    if (m_raw)
    {
        // setting port does not apply for raw servers
        return;
    }

    if (!m_transporter)
    {
        pd_error(m_x, "no transporter!");
        return;
    }

    // check upper limit
    if (port > (int)UINT16_MAX)
    {
        pd_error(m_x, "invalid port: %d", port);
        return;
    }

    if (m_transporter->isListening() &&
        m_transporter->port() == port)
    {
        // port not changed
        return;
    }

    // stop listening
    m_transporter->unbind();

    if (port > 0)
    {
        m_transporter->bind(port);
    }
}

size_t ParameterServer::clientCount() const
{
    if (m_transporter)
    {
        return m_transporter->clientCount();
    }

    return 0;
}

// parameter
void ParameterServer::exposeParameter(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    // <type> <group> <group> ... <label>
    // options: @min @max @readonly @order

    if (argc < 2)
    {
        pd_error(m_x, "not enough arguments to expose parameter");
        return;
    }

    // check type
    rcp_datatype datatype = DATATYPE_INVALID;
    if (argv[0].a_type != A_SYMBOL)
    {
        pd_error(m_x, "please provide a valid type (f, i, t, b, s)");
        return;
    }

    //
    const char* type_str = argv[0].a_w.w_symbol->s_name;

    if (*type_str == 'f' || *type_str == 'F')
    {
        // TODO check pd/max float size!
        datatype = DATATYPE_FLOAT32;
    }
    else if (*type_str == 'i' || *type_str == 'I')
    {
        datatype = DATATYPE_INT32;
    }
    else if (*type_str == 't' || *type_str == 'T')
    {
        datatype = DATATYPE_BOOLEAN;
    }
    else if (*type_str == 'b' || *type_str == 'B')
    {
        datatype = DATATYPE_BANG;
    }
    else if (*type_str == 's' || *type_str == 'S')
    {
        datatype = DATATYPE_STRING;
    }


    // check datatype
    if (datatype == DATATYPE_INVALID)
    {
        pd_error(m_x, "unknown datatype: %s", type_str);
        return;
    }


    // arguments
    Optional<float> min;
    Optional<float> max;
    Optional<bool> readonly;
    Optional<int> order;

    // get options - look for first atom starting with @
    int args_index = argc;
    for (int i=2; i<argc; i++)
    {
        if (argv[i].a_type == A_SYMBOL)
        {
            std::string t = argv[i].a_w.w_symbol->s_name;
            if (t[0] == '@')
            {
                // set first arguments index
                if (args_index == argc) args_index = i;


                if (t == "@min")
                {
                    i++;
                    if (i >= argc) break;

                    if (canBeFloat(argv[i]))
                    {
                        min.set(argv[i].a_w.w_float);
                    }
                    else
                    {
                        // error
                        pd_error(m_x, "can not set argument min!");
                    }
                }
                else if (t == "@max")
                {
                    i++;
                    if (i >= argc) break;

                    if (canBeFloat(argv[i]))
                    {
                        max.set(argv[i].a_w.w_float);
                    }
                    else
                    {
                        // error
                        pd_error(m_x, "can not set argument max!");
                    }
                }
                else if (t == "@order")
                {
                    i++;
                    if (i >= argc) break;

                    if (canBeInt(argv[i]))
                    {
                        order.set(getInt(argv[i]));
                    }
                    else
                    {
                        // error
                        pd_error(m_x, "can not set argument order!");
                    }
                }
                else if (t == "@readonly")
                {
                    readonly.set(true);
                }
            }
        }
    }

    // create necessary groups
    std::string label;
    rcp_group_parameter* group = createGroups(args_index-1, argv+1, label);

    if (label.empty())
    {
        // no label - can not create parameter
        pd_error(m_x, "please provide a label to expose a parameter");
        return;
    }

    // check if this label already exists in group
    rcp_parameter* param = rcp_manager_find_parameter(m_manager, label.c_str(), group);
    if (param != NULL)
    {
        pd_error(m_x, "parameter '%s' already exists", label.c_str());
        return;
    }


    // expose parameter
    switch (datatype)
    {
    case DATATYPE_FLOAT32:
    {
        rcp_value_parameter* p = rcp_server_expose_f32(m_server, label.c_str(), group);
        setupValueParameter(p);
        if (p != NULL)
        {
            if (min.isSet()) rcp_parameter_set_min_float(p, min.get());
            if (max.isSet()) rcp_parameter_set_max_float(p, max.get());
            if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
            if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get());

            // set default value
            rcp_parameter_set_value_float(p, 0);
        }
        else
        {
            pd_error(m_x, "could not create parameter");
        }
        break;
    }
    case DATATYPE_INT32:
    {
        rcp_value_parameter* p = rcp_server_expose_i32(m_server, label.c_str(), group);
        setupValueParameter(p);
        if (p != NULL)
        {
            if (min.isSet()) rcp_parameter_set_min_int32(p, (int32_t)min.get());
            if (max.isSet()) rcp_parameter_set_max_int32(p, (int32_t)max.get());
            if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
            if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get());

            // set default value
            rcp_parameter_set_value_int32(p, 0);
        }
        else
        {
            pd_error(m_x, "could not create parameter");
        }
        break;
    }
    case DATATYPE_BOOLEAN:
    {
        rcp_value_parameter* p = rcp_server_expose_bool(m_server, label.c_str(), group);
        setupValueParameter(p);
        if (p != NULL)
        {
            if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
            if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get());

            // set default value
            rcp_parameter_set_value_bool(p, false);
        }
        else
        {
            pd_error(m_x, "could not create parameter");
        }
        break;
    }
    case DATATYPE_STRING:
    {
        rcp_value_parameter* p = rcp_server_expose_string(m_server, label.c_str(), group);
        setupValueParameter(p);
        if (p != NULL)
        {
            if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
            if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get());

            // set default value
            rcp_parameter_set_value_string(p, "");
        }
        else
        {
            pd_error(m_x, "could not create parameter");
        }
        break;
    }
    case DATATYPE_BANG:
    {
        rcp_bang_parameter* p = rcp_server_expose_bang(m_server, label.c_str(), group);
        if (p != NULL)
        {
            if (order.isSet()) rcp_parameter_set_order(RCP_PARAMETER(p), order.get());
            if (readonly.isSet()) rcp_parameter_set_readonly(RCP_PARAMETER(p), readonly.get());

            rcp_parameter_set_user(RCP_PARAMETER(p), this);
            rcp_bang_parameter_set_bang_cb(p, bangCb);
        }
        else
        {
            pd_error(m_x, "could not create parameter");
        }
        break;
    }
    }

    rcp_server_update(m_server);
}


rcp_group_parameter* ParameterServer::createGroups(int argc, t_atom* argv, std::string& outLabel)
{
    rcp_group_parameter* lastGroup = nullptr;
    for (int i=0; i<argc-1; i++)
    {
        if (argv[i].a_type != A_SYMBOL)
        {
            //
            pd_error(m_x, "can not create group with non-string name");
            outLabel = "";
            return nullptr;
        }

        std::string group_name = GetAsString(argv[i]);
        rcp_group_parameter* group = rcp_server_find_group(m_server, group_name.c_str(), lastGroup);

        if (group == nullptr)
        {
            // create group
            group = rcp_server_create_group(m_server, group_name.c_str(), lastGroup);
        }

        lastGroup = group;
    }

    outLabel = GetAsString(argv[argc-1]);
    return lastGroup;
}

void ParameterServer::setupValueParameter(rcp_value_parameter* parameter)
{
    if (parameter)
    {
        rcp_parameter_set_user(RCP_PARAMETER(parameter), this);
        rcp_parameter_set_value_updated_cb(parameter, parameterValueUpdatedCb);
    }
}

void ParameterServer::removeParameter(int id)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (rcp_server_remove_parameter_id(m_server, id))
    {
        rcp_server_update(m_server);
    }
}

void ParameterServer::removeParameterList(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* param = getParameter(argc, argv);
    if (param)
    {
        removeParameter(rcp_parameter_get_id(param));
    }
}

void ParameterServer::parameterSetReadonly(int argc, t_atom* argv)
{    
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (!canBeInt(argv[argc-1]))
    {
        pd_error(m_x, "rcp set readonly - invalid data");
        return;
    }

    rcp_parameter* parameter = getParameter(argc-1, argv);
    if (parameter)
    {
        rcp_parameter_set_readonly(parameter, getInt(argv[argc-1]) > 0);
        rcp_server_update(m_server);
    }
}

void ParameterServer::parameterSetOrder(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (!canBeInt(argv[argc-1]))
    {
        pd_error(m_x, "rcp set order - invalid data");
        return;
    }

    rcp_parameter* parameter = getParameter(argc-1, argv);
    if (parameter)
    {
        rcp_parameter_set_order(parameter, getInt(argv[argc-1]));
        rcp_server_update(m_server);
    }
}

void ParameterServer::parameterSetMin(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc-1, argv);
    if (parameter)
    {
        rcp_datatype type = RCP_TYPE_ID(parameter);
        if (type == DATATYPE_FLOAT32)
        {
            if (canBeFloat(argv[argc-1]))
            {
                rcp_parameter_set_min_float(RCP_VALUE_PARAMETER(parameter), argv[argc-1].a_w.w_float);
                rcp_server_update(m_server);
            }
        }
        else if (type == DATATYPE_INT32)
        {
            if (canBeInt(argv[argc-1]))
            {
                rcp_parameter_set_min_int32(RCP_VALUE_PARAMETER(parameter), getInt(argv[argc-1]));
                rcp_server_update(m_server);
            }
        }
    }
}

void ParameterServer::parameterSetMax(int argc, t_atom* argv)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc-1, argv);
    if (parameter)
    {
        rcp_datatype type = RCP_TYPE_ID(parameter);
        if (type == DATATYPE_FLOAT32)
        {
            if (canBeFloat(argv[argc-1]))
            {
                rcp_parameter_set_max_float(RCP_VALUE_PARAMETER(parameter), argv[argc-1].a_w.w_float);
                rcp_server_update(m_server);
            }
        }
        else if (type == DATATYPE_INT32)
        {
            if (canBeInt(argv[argc-1]))
            {
                rcp_parameter_set_max_int32(RCP_VALUE_PARAMETER(parameter), getInt(argv[argc-1]));
                rcp_server_update(m_server);
            }
        }
    }
}

void ParameterServer::parameterSetMinMax(int argc, t_atom* argv)
{
    if (argc < 3 ||
        argv[argc-2].a_type != A_FLOAT ||
        argv[argc-1].a_type != A_FLOAT)
    {
        pd_error(m_x, "invalid data for setting min and max");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    rcp_parameter* parameter = getParameter(argc-2, argv);
    if (parameter)
    {
        rcp_datatype type = RCP_TYPE_ID(parameter);
        if (type == DATATYPE_FLOAT32)
        {
            rcp_parameter_set_min_float(RCP_VALUE_PARAMETER(parameter), argv[argc-2].a_w.w_float);
            rcp_parameter_set_max_float(RCP_VALUE_PARAMETER(parameter), argv[argc-1].a_w.w_float);
            rcp_server_update(m_server);
        }
        else if (type == DATATYPE_INT32)
        {
            rcp_parameter_set_min_int32(RCP_VALUE_PARAMETER(parameter), getInt(argv[argc-2]));
            rcp_parameter_set_max_int32(RCP_VALUE_PARAMETER(parameter), getInt(argv[argc-1]));
            rcp_server_update(m_server);
        }
    }
}

// rabbithole
void ParameterServer::setRabbithole(const std::string& uri)
{
    std::lock_guard<std::recursive_mutex> lock(Threading::mutex);

    if (!uri.empty() &&
        uri.rfind("ws", 0) != 0 &&
        uri.rfind("http", 0) != 0)
    {
        pd_error(m_x, "invalid uri for rabbithole");
        return;
    }

    if (m_rabbitholeTransporter)
    {
        m_rabbitholeTransporter.reset();
    }

    if (!uri.empty())
    {
        if (m_server)
        {
            m_rabbitholeTransporter = std::make_shared<RabbitHoleServerTransporter>((t_pd*)m_x, m_server);
        }

        if (m_rabbitholeTransporter)
        {
            m_rabbitholeTransporter->connect(uri);
        }
        else
        {
            pd_error(m_x, "could not create rabbithole transporter");
        }
    }
}

void ParameterServer::setRabbitholeInterval(const int i)
{
    if (m_rabbitholeTransporter)
    {
        m_rabbitholeTransporter->setInterval(i);
    }
}

void ParameterServer::handle_raw_data(char* data, size_t size)
{
    if (m_transporter)
    {
        m_transporter->pushData(data, size);
    }
}

} // namespace rcp
