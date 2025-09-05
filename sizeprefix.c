#include <string.h>

#if defined(__APPLE__)
#include <arpa/inet.h>
#elif defined(__linux__)
#include <endian.h>
#elif defined(_WIN32)
// TODO
#endif


#include <m_pd.h>

typedef struct _sizeprefix
{
    t_object x_obj;

    t_outlet* list_out;
    t_outlet* prefix_out;

} t_sizeprefix;

static t_class *sizeprefix_class;


// pd interfacea

void sizeprefix_list(t_sizeprefix *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom size_a[4];
    char size_c[4];

#if defined(__APPLE__)
    uint32_t n = htonl(argc);
#elif defined(__linux__)
    uint32_t n = htobe32(argc);
#elif defined(_WIN32)
    // TODO
#endif

    memcpy(size_c, &n, sizeof(uint32_t));

    for (int i=0; i<4; i++)
    {
#ifdef PD_MAJOR_VERSION
        size_a[i].a_type = A_FLOAT;
        size_a[i].a_w.w_float = (float)size_c[i];
#else
        size_a[i].a_type = A_INT;
        size_a[i].a_w.w_long = size_c[i];
#endif
    }

    outlet_list(x->prefix_out, NULL, 4, size_a);
    outlet_list(x->list_out, NULL, argc, argv);
}



// new

void *sizeprefix_new(t_symbol *s)
{
    t_sizeprefix *x = (t_sizeprefix *)pd_new(sizeprefix_class);

    x->list_out = outlet_new(&x->x_obj, &s_list);
    x->prefix_out = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}


void sizeprefix_free(t_sizeprefix *x)
{
    outlet_free(x->list_out);
    outlet_free(x->prefix_out);
}

void sizeprefix_setup(void) {
    sizeprefix_class = class_new(gensym("sizeprefix"),
                                   (t_newmethod)sizeprefix_new,
                                   (t_method)sizeprefix_free,
                                   sizeof(t_sizeprefix),
                                   CLASS_DEFAULT,
                                   A_NULL,
                                   0);


    class_addlist(sizeprefix_class, (t_method)sizeprefix_list);
}

