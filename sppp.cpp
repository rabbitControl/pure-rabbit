#include <string.h>

#include <m_pd.h>
#include <rcp_sppp.h>

#include "PdMaxUtils.h"

using namespace PdMaxUtils;

#ifdef __cplusplus
extern "C"{
#endif


typedef struct _sppp
{
    t_object x_obj;

    rcp_sppp* parser;

    t_outlet* list_out;

} t_sppp;

static t_class *sppp_class;



static void packet_cb(const char* data, size_t data_size, void* user)
{
    if (user == NULL)
    {
        return;
    }

    t_sppp* x = (t_sppp*)user;

    t_atom* atoms = new t_atom[data_size];

    for (size_t i=0; i<data_size; i++)
    {
        setInt(atoms[i], data[i]);
    }

    outlet_list(x->list_out, NULL, data_size, atoms);

    delete [] atoms;
}


void sppp_float(t_sppp *x, float f)
{
    int data = (int)f;
    if (data < 256 && data >= 0)
    {
        char d = (char)data;
        rcp_sppp_data(x->parser, &d, 1);
    }
}


void sppp_list(t_sppp *x, t_symbol *s, int argc, t_atom *argv)
{
    char* data = new char[argc];
    int offset = 0;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            int id = (int)getAInt(argv[i], -1);
            if (id >= 0 &&
                id < 256)
            {
                data[i-offset] = id;
            }
            else
            {
                offset++;
            }
        }
        else
        {
            offset++;
        }
    }

    rcp_sppp_data(x->parser, data, argc - offset);

    delete [] data;
}


void sppp_reset(t_sppp* x)
{
    rcp_sppp_reset(x->parser);
}


void *sppp_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sppp *x = (t_sppp *)pd_new(sppp_class);

    x->list_out = outlet_new(&x->x_obj, &s_list);


    int buffer_size = 1024;

    for (int i=0; i<argc; i++)
    {
        if (canBeInt(argv[i]))
        {
            buffer_size = getAInt(argv[i], 1024);
        }
    }

    if (buffer_size <= 0)
    {
        pd_error(x, "please provide a valid buffersize");
        buffer_size = 1024;
    }

    // create parser
    x->parser = rcp_sppp_create(buffer_size, packet_cb, x);


    return (void *)x;
}


void sppp_free(t_sppp *x)
{
    if (x->parser)
    {
        rcp_sppp_free(x->parser);
    }

    outlet_free(x->list_out);
}


void sppp_setup(void) {
    sppp_class = class_new(gensym("sppp"),
                           (t_newmethod)sppp_new,
                           (t_method)sppp_free,
                           sizeof(t_sppp),
                           CLASS_DEFAULT,
                           A_GIMME,
                           0);


    class_addfloat(sppp_class, (t_method)sppp_float);
    class_addlist(sppp_class, (t_method)sppp_list);

    class_addmethod(sppp_class, (t_method)sppp_reset, gensym("reset"), A_NULL, A_NULL);
}


#ifdef __cplusplus
} // extern "C"
#endif
