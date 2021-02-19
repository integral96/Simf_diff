#include <iostream>
#include <sstream>

#include <boost/hana.hpp>
#include <boost/hana/assert.hpp>

#include "simvolic_diff.hpp"

namespace hana = boost::hana;

using namespace hana::literals;
using namespace std::literals;

constexpr auto A = hana::make_set(hana::int_c<1>, hana::int_c<3>, hana::int_c<5>, hana::int_c<7>);
constexpr auto B = hana::make_set(hana::int_c<2>, hana::int_c<4>, hana::int_c<6>);

constexpr auto U = hana::union_(A, B);

constexpr auto result = hana::unpack(U, [](auto... x){
    return hana::make_set(hana::make_pair(9 - x, hana::max(x, 9 - x) == x ? x : 0)...);
});
constexpr auto result1 = hana::unpack(U, [](auto... x){
    return hana::make_tuple(hana::make_pair(x,  8 - x)...);
});

using type_A = mpl::vector4_c<int, 1, 3, 5, 7>;
using type_B = mpl::vector3_c<int, 2, 4, 6>;

template<int N, int M>
struct pair {

    static constexpr int first = mpl::at_c<type_A, N>::type::value;
    static constexpr int second = mpl::at_c<type_B, M>::type::value;
};

template<int N, int M>
struct U_st : mpl::if_c<(mpl::plus<typename mpl::at_c<type_A, N>::type, typename mpl::at_c<type_B, M>::type>::type::value == 9),
                        pair<N, M>, mpl::na>::type {

};

#define eps 0.000001
double fx(double x) { return x*x + x;} // вычисляемая функция
double dfx(double x) { return 2*x + 1;} // производная функции

typedef double(*function)(double x); // задание типа function

double solve(function fx, function dfx, double x0) {
  double x1  = x0 - fx(x0)/dfx(x0); // первое приближение

  while (std::abs(x1-x0)>eps) { // пока не достигнута точность 0.000001
    x0 = x1;
    x1 = x0 - fx(x0)/dfx(x0); // последующие приближения
  }
  return x1;
}

auto result2 = hana::sort(result1, hana::less);
int main()
{
    std::stringstream ss2;
    hana::for_each(result, [&](auto x) {
        if(hana::first(x) + hana::second(x) == 9)
        ss2 << hana::first(x) << " " << hana::second(x) << "\n";
    });

    std::stringstream ss3;
    hana::for_each(result2, [&](auto x) {
//        if(hana::first(x) < hana::second(x))
        ss3 << hana::first(x) << " " << hana::second(x) << "\n";
    });

    std::cout << ss2.str() << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << ss3.str() << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << U_st<3, 0>::first << std::endl;


    variable x;
    double result = newton(x*x + x , 1.1, 1000);
    std::cout << result << '\n';
    std::cout << "============================" << std::endl;
    std::cout << solve(fx, dfx, 1.1) << '\n';

    return 0;
}
