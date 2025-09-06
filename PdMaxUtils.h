#ifndef PD_MAX_UTILS_H
#define PD_MAX_UTILS_H

#include <m_pd.h>

namespace PdMaxUtils {


#ifdef PD_MAJOR_VERSION

// PD

static void setSymbol(t_atom& a,const t_symbol* s)
{
    a.a_type = A_SYMBOL;
    a.a_w.w_symbol = const_cast<t_symbol *>(s);
}

static float getAFloat(const t_atom& a, float def)
{
    return a.a_type == A_FLOAT ? a.a_w.w_float : def;
}

static bool isInt(const t_atom &)
{
    return false;
}

static int getInt(const t_atom &a)
{
    return (int)a.a_w.w_float;
}

static void setInt(t_atom& a, int v)
{
    a.a_type = A_FLOAT;
    a.a_w.w_float = (float)v;
}

#else

// Max

static void setSymbol(t_atom& a, const t_symbol* s)
{
    a.a_type = A_SYMBOL;
    a.a_w.w_sym = const_cast<t_symbol *>(s);
}


static bool isInt(const t_atom& a)
{
    return a.a_type == A_INT;
}

static int getInt(const t_atom& a)
{
    return a.a_w.w_long;
}

static void setInt(t_atom& a, int v)
{
    a.a_type = A_INT;
    a.a_w.w_long = v;
}

static float getAFloat(const t_atom& a, float def = 0)
{
    return a.a_type == A_FLOAT ? a.a_w.w_float : (isInt(a) ? a->a_w.w_long : def));
}

#endif


static void setString(t_atom& a, const char* c)
{
    setSymbol(a, gensym(c));
}

static bool canBeInt(const t_atom& a)
{
    return a.a_type == A_FLOAT || isInt(a);
}

static bool canBeFloat(const t_atom& a)
{
    return a.a_type == A_FLOAT || isInt(a);
}

static void setFloat(t_atom& a,float v)
{
    a.a_type = A_FLOAT;
    a.a_w.w_float = v;
}

static int getAInt(const t_atom& a, int def)
{
    return (int)getAFloat(a,(float)def);
}

}

#endif // PD_MAX_UTILS_H
