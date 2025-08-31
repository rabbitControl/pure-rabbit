#ifndef OPTIONAL_H
#define OPTIONAL_H

template <class type>
class Optional
{
public:
    Optional() {}

    void set(const type& v)
    {
        m_value = v;
        m_valueSet = true;
    }
    type get() const
    {
        return m_value;
    }
    void clear()
    {
        m_valueSet = false;
    }
    bool isSet() const
    {
        return m_valueSet;
    }

private:
    type m_value;
    bool m_valueSet{false};
};

#endif // OPTIONAL_H
