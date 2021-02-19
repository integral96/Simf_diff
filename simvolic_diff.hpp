#ifndef SIMVOLIC_DIFF_HPP
#define SIMVOLIC_DIFF_HPP

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/for_each.hpp>

#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;

template<class O>
struct math_base {
    O& self() {
        return static_cast<O&>(*this);
    }
    const O& self() const {
        return static_cast<const O&>(*this);
    }
};
template<class E>
struct expression : math_base<E> {};
template<int N>
struct int_constant : expression<int_constant<N>> {
    static constexpr int value = N;
    using diff_type = int_constant<0>;

    diff_type diff() const {
        return diff_type();
    }
    template<typename T>
    int operator()(const T&) const {
        return value;
    }
};
template<typename E>
struct is_int_constant : mpl::false_ {};
template<int N>
struct is_int_constant<int_constant<N>> : mpl::true_ {};
template<typename E>
struct int_constant_value : std::integral_constant<int, 0> {};
template<int N>
struct int_constant_value<int_constant<N>> : std::integral_constant<int, N> {};

template<typename VT>
struct scalar : expression<scalar<VT>> {
    using value_type = VT;
    using diff_type = int_constant<0>;
    const value_type value;
    scalar(const value_type& value) : value(value) {}
    diff_type diff() const {
        return diff_type();
    }
    template<typename T>
    value_type operator()(const T& x) const {
        return 0;
    }
};
template<class E>
struct is_scalar : mpl::false_ {};
template<typename VT>
struct is_scalar<scalar<VT>> : mpl::true_ {};

template<typename T>
scalar<T> _(const T& val) {
    return scalar<T>(val);
}
struct variable : expression<variable> {
    using diff_type = int_constant<1>;
    diff_type diff() const {
        return diff_type();
    }
    template<typename T>
    T operator()(const T& x) const {
        return x;
    }
};
template<class E>
struct negate_expression;
template<class E>
struct negate_expression_type : mpl::if_c<is_int_constant<E>::value, int_constant<-int_constant_value<E>::value>,
                                typename mpl::if_c<is_scalar<E>::value, E, negate_expression<E>>::type> {};

template<class E>
struct negate_expression : expression<negate_expression<E>> {
    using diff_type = typename negate_expression_type<typename E::diff_type>::type;
    const E e;
    negate_expression(const expression<E>& e) : e(e.self()) {}
    diff_type diff() const {
        return -e.diff();
    }
    template<typename T>
    T operator()(const T& x) const {
        return -e(x);
    }
};
template<class E>
negate_expression<E> operator-(const expression<E>& e) {
    return negate_expression<E>(e);
}
template<int N>
int_constant<-N> operator-(const int_constant<N>&) {
    return int_constant<-N>();
}
template<typename VT>
scalar<VT> operator-(const scalar<VT>& e) {
    return scalar<VT>(-e.value);
}

template<class E1, char Op, class E2>
struct additive_expression;
template<class E1, char Op, class E2>
struct additive_expression_type : mpl::if_c<(is_int_constant<E1>::value && is_int_constant<E2>::value && Op == '+'),
                                int_constant<int_constant_value<E1>::value + int_constant_value<E2>::value>,
                                typename mpl::if_c<(is_int_constant<E1>::value && is_int_constant<E2>::value && Op == '-'),
                                int_constant<int_constant_value<E1>::value - int_constant_value<E2>::value>,
                                typename mpl::if_c<(is_int_constant<E1>::value && is_scalar<E2>::value && Op == '+'),
                                int_constant<int_constant_value<E1>::value>,
                                typename mpl::if_c<(is_int_constant<E1>::value && is_scalar<E2>::value && Op == '-'),
                                int_constant<int_constant_value<E1>::value>,
                                additive_expression<E1, Op, E2>>::type>::type>::type> {};

template<char Op, class E1, class E2>
typename std::enable_if_t<Op == '+', typename additive_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type>
additive_expression_diff(const E1& e1, const E2& e2) {
    return e1.diff() + e2.diff();
}
template<char Op, class E1, class E2>
typename std::enable_if_t<Op == '-', typename additive_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type>
additive_expression_diff(const E1& e1, const E2& e2) {
    return e1.diff() - e2.diff();
}

template<class E1, char Op, class E2>
struct additive_expression : expression<additive_expression<E1, Op, E2>> {
    using diff_type = typename additive_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type;
    const E1 e1;
    const E2 e2;
    additive_expression(const expression<E1>& e1, const expression<E2>& e2) : e1(e1.self()), e2(e2.self()) {
    }
    diff_type diff() const {
        return additive_expression_diff<Op>(e1, e2);
    }
    template<typename T>
    typename std::enable_if_t<Op == '+', T>
    operator()(const T &x) const {
        return e1(x) + e2(x);
    }
    template<typename T>
    typename std::enable_if_t<Op == '-', T>
    operator()(const T &x) const {
        return e1(x) - e2(x);
    }
};

template<class E1, class E2>
additive_expression<E1, '+', E2> operator+(const expression<E1>& e1, const expression<E2>& e2) {
    return additive_expression<E1, '+', E2>(e1, e2);
}
template<class E1, class E2>
additive_expression<E1, '-', E2> operator-(const expression<E1>& e1, const expression<E2>& e2) {
    return additive_expression<E1, '-', E2>(e1, e2);
}

template<class E>
const E& operator+(const expression<E> &e, const int_constant<0>&) {
    return e.self();
}
template<class E>
const E& operator+(const int_constant<0>&, const expression<E> &e) {
    return e.self();
}
template<class E>
const E& operator-(const int_constant<0>&, const expression<E> &e) {
    return -e.self();
}
template<class E>
const E& operator-(const expression<E> &e, const int_constant<0>&) {
    return e.self();
}
template<int N1, int N2>
int_constant<N1 + N2> operator+(const int_constant<N1>&, const int_constant<N2>&){
    return int_constant<N1 + N2>();
}
template<int N1>
int_constant<N1> operator+(const int_constant<N1>&, const int_constant<0>&){
    return int_constant<N1>();
}
template<int N2>
int_constant<N2> operator+(const int_constant<0>&, const int_constant<N2>&){
    return int_constant<N2>();
}
template<int N1, int N2>
int_constant<N1 - N2> operator-(const int_constant<N1>&, const int_constant<N2>&){
    return int_constant<N1 - N2>();
}
template<int N1>
int_constant<N1> operator-(const int_constant<N1>&, const int_constant<0>&){
    return int_constant<N1>();
}
template<int N2>
int_constant<N2> operator-(const int_constant<0>&, const int_constant<N2>&){
    return int_constant<-N2>();
}
template<typename VT>
scalar<VT> operator+(const scalar<VT>& e1, const scalar<VT>& e2) {
    return scalar<VT>(e1.value + e2.value);
}
template<typename VT>
scalar<VT> operator-(const scalar<VT>& e1, const scalar<VT>& e2) {
    return scalar<VT>(e1.value - e2.value);
}
template<typename T>
struct is_variable : mpl::false_ {};
template<>
struct is_variable<variable> : mpl::true_ {};

template<class E1, class E2>
struct multvar_type : std::enable_if<(is_int_constant<E1>::value && is_variable<E2>::value),
                                 decltype (std::declval<E1>()*variable())> {};

template<class E1, char Op, class E2>
struct multy_expression;
template<class E1, char Op, class E2>
struct multy_expression_type : mpl::if_c<(is_int_constant<E1>::value && is_int_constant<E2>::value && Op == '*'),
                                additive_expression<std::decay_t<decltype (std::declval<E1>()*variable())>, '+', decltype (std::declval<E2>()*variable())>,
                                multy_expression<E1, '*', E2>
                                > {};


template<char Op, class E1, class E2>
typename std::enable_if_t<Op == '*', typename multy_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type>
multy_expression_diff(const E1& e1, const E2& e2) {
    return e1.diff()*e2.self() + e2.diff()*e1.self();
}
template<char Op, class E1, class E2>
typename std::enable_if_t<Op == '/', typename multy_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type>
multy_expression_diff(const E1& e1, const E2& e2) {
    return (e1.diff()*e2.self() - e2.diff()*e1.self())/(e2.self()*e2.self());
}

template<class E1, char Op, class E2>
struct multy_expression : expression<multy_expression<E1, Op, E2>> {
    using diff_type = typename multy_expression_type<typename E1::diff_type, Op, typename E2::diff_type>::type;
    const E1 e1;
    const E2 e2;
    multy_expression(const expression<E1>& e1, const expression<E2>& e2) : e1(e1.self()), e2(e2.self()) {
    }
    diff_type diff() const {
        return multy_expression_diff<Op>(e1, e2);
    }
    template<typename T>
    typename std::enable_if_t<Op == '*', T>
    operator()(const T &x) const {
        return e1(x) * e2(x);
    }
    template<typename T>
    typename std::enable_if_t<Op == '/', T>
    operator()(const T &x) const {
        return e1(x) / e2(x);
    }
};
template<class E1, class E2>
multy_expression<E1, '*', E2> operator*(const expression<E1>& e1, const expression<E2>& e2) {
    return multy_expression<E1, '*', E2>(e1, e2);
}
template<class E1, class E2>
additive_expression<E1, '/', E2> operator/(const expression<E1>& e1, const expression<E2>& e2) {
    return additive_expression<E1, '/', E2>(e1, e2);
}

template<class E>
const int_constant<0>& operator*(const expression<E> &e, const int_constant<0>&) {
    return int_constant<0>();
}
template<class E>
const int_constant<0>& operator*(const int_constant<0>&, const expression<E> &e) {
    return int_constant<0>();
}
template<int N1, int N2>
int_constant<N1 * N2> operator*(const int_constant<N1>&, const int_constant<N2>&){
    return int_constant<N1 * N2>();
}
template<int N1>
int_constant<0> operator*(const int_constant<N1>&, const int_constant<0>&){
    return int_constant<0>();
}
template<int N2>
int_constant<0> operator*(const int_constant<0>&, const int_constant<N2>&){
    return int_constant<0>();
}

template<class E>
double newton(const expression<E> &expr, double x, unsigned maxiter){
    const E &f = expr.self();
    const typename E::diff_type fd = f.diff();
    const double eps = 1e-12;
    unsigned niter = 0;
    double fx;
    while(std::abs(fx = f(x)) > eps) {
        std::cout << x << std::endl;
        if(++niter > maxiter) throw std::runtime_error("Too many iterations");
        x -= fx / fd(x);
    }
    return x;
}
#endif // SIMVOLIC_DIFF_HPP
