#include "amc.h"

using std::optional;
using std::vector;

bool expect_color(optional<ColorCount> const &v, Color_ col)
{
    if (!v.has_value())
    {
        std::cout << "No value available.\n";
        return false;
    }
    if (v->color != col)
    {
        std::cout << "Expected " << col << " got " << v->color << ".\n";
        return false;
    }
    return true;
}
bool test_aas(UuAasModeColorSolver const &a, AxisAlignedSquare const &aas, Color_ col)
{
    auto res = a.a_mode_color(aas);
    return expect_color(res, col);
}

bool check_solver(UuAasModeColorSolver const &solver)
{
    solver.set_statistics_gathering(false);

    bool all_ok = true;
    all_ok = all_ok && test_aas(solver, AxisAlignedSquare(0., 0., 10.), 2);
    all_ok = all_ok && test_aas(solver, AxisAlignedSquare(10., 10., 2.), 1);
    return all_ok;
}

bool check_all_solvers()
{
    vector<ColoredPoint_2> points{{9., 9., 1}, {-5., 4., 2}, {-3., -1., 2}};
    bool all_ok = true;

    std::clog << "grid_select_colors_solver ...\n";
    all_ok = all_ok && check_solver(*grid_select_colors_solver(points, 8));
    std::clog << "grid_all_colors_solver ...\n";
    all_ok = all_ok && check_solver(*grid_all_colors_solver(points, 8));
    std::clog << "grid_sorted_colors_solver ...\n";
    all_ok = all_ok && check_solver(*grid_sorted_colors_solver(points, 8));
    std::clog << "grid_select_sorted_colors_solver ...\n";
    all_ok = all_ok && check_solver(*grid_select_sorted_colors_solver(points, 8));
    std::clog << "grid_all_points_solver ...\n";
    all_ok = all_ok && check_solver(*grid_all_points_solver(points, 8));

    return all_ok;
}
int main()
{
    return check_all_solvers() ? 0 : 1;
}