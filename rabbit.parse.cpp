#include "rabbit.parse.h"

#include "RcpParse.h"

#ifdef __cplusplus
extern "C"{
#endif

static t_class *rcp_parse_class;


// pd interfacea

void rcpparse_list(t_rabbit_parse_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->parse)
    {
        x->parse->handleList(argc, argv);
    }
}

// new

void *rcpparse_pd_new(t_symbol *s)
{
    t_rabbit_parse_pd *x = (t_rabbit_parse_pd *)pd_new(rcp_parse_class);

    x->parse = new RcpParse(x);

    return (void *)x;
}


void rcpparse_pd_free(t_rabbit_parse_pd *x)
{
    if (x->parse)
    {
        delete x->parse;
    }
}

void setup_rabbit0x2eparse(void) {
    rcp_parse_class = class_new(gensym("rabbit.parse"),
                                   (t_newmethod)rcpparse_pd_new,
                                   (t_method)rcpparse_pd_free,
                                   sizeof(t_rabbit_parse_pd),
                                   CLASS_DEFAULT,
                                   A_NULL,
                                   0);


    class_addlist(rcp_parse_class, (t_method)rcpparse_list);
}

#ifdef __cplusplus
} // extern "C"
#endif
