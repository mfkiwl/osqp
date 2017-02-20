#include "osqp.h"     // OSQP API
// #include "cs.h"       // CSC data structure
// #include "util.h"     // Utilities for testing
#include "minunit.h"  // Basic testing script header

#ifndef BASIC_QP2_MATRICES_H
#define BASIC_QP2_MATRICES_H
#include "basic_qp2/basic_qp2.h"
#endif



static char * test_basic_qp2_solve()
{
    /* local variables */
    c_int exitflag = 0;  // No errors

    // Problem settings
    Settings * settings = (Settings *)c_malloc(sizeof(Settings));

    // Structures
    Work * work;  // Workspace
    Data * data;  // Data
    basic_qp2_sols_data *  sols_data = generate_problem_basic_qp2_sols_data();


    // Populate data
    data = generate_problem_basic_qp();
    sols_data = generate_problem_basic_qp2_sols_data();


    // Define Solver settings as default
    set_default_settings(settings);
    settings->max_iter = 1000;
    settings->alpha = 1.6;
    settings->warm_start = 1;
    settings->polishing = 1;
    settings->verbose = 0;

    // Setup workspace
    work = osqp_setup(data, settings);

    // Setup correct
    mu_assert("Basic QP 2 test solve: Setup error!", work != OSQP_NULL);

    // Solve Problem first time
    osqp_solve(work);

    // Compare solver statuses
    mu_assert("Basic QP 2 test solve: Error in solver status!",
              work->info->status_val == sols_data->status_test );

    // Compare primal solutions
    mu_assert("Basic QP 2 test solve: Error in primal solution!",
              vec_norm2_diff(work->solution->x, sols_data->x_test, data->n) < TESTS_TOL);

    // Compare dual solutions
    mu_assert("Basic QP 2 test solve: Error in dual solution!",
              vec_norm2_diff(work->solution->y, sols_data->y_test, data->m) < TESTS_TOL);


    // Compare objective values
    mu_assert("Basic QP 2 test solve: Error in objective value!",
              c_absval(work->info->obj_val - sols_data->obj_value_test) < TESTS_TOL);


    // Clean workspace
    osqp_cleanup(work);

    // Cleanup settings and data
    c_free(settings);
    clean_problem_basic_qp2(data);
    clean_problem_basic_qp2_sols_data(sols_data);

    return 0;
}


static char * test_basic_qp2_update()
{
    /* local variables */
    c_int exitflag = 0;  // No errors

    // Problem settings
    Settings * settings = (Settings *)c_malloc(sizeof(Settings));

    // Structures
    Work * work;  // Workspace
    Data * data;  // Data
    basic_qp2_sols_data *  sols_data = generate_problem_basic_qp2_sols_data();


    // Populate data
    data = generate_problem_basic_qp();
    sols_data = generate_problem_basic_qp2_sols_data();


    // Define Solver settings as default
    set_default_settings(settings);
    settings->max_iter = 1000;
    settings->alpha = 1.6;
    settings->warm_start = 1;
    settings->polishing = 1;
    settings->verbose = 0;

    // Setup workspace
    work = osqp_setup(data, settings);

    // Setup correct
    mu_assert("Basic QP 2 test update: Setup error!", work != OSQP_NULL);


    // Modify linear cost and upper bound
    osqp_update_lin_cost(work, sols_data->q_new);
    osqp_update_upper_bound(work, sols_data->u_new);

    // Solve Problem second time(with different data now)
    osqp_solve(work);

    // Compare solver statuses
    mu_assert("Basic QP 2 test update: Error in solver status!",
              work->info->status_val == sols_data->status_test_new );

    // Compare primal solutions
    mu_assert("Basic QP 2 test update: Error in primal solution!",
              vec_norm2_diff(work->solution->x, sols_data->x_test_new, data->n) < TESTS_TOL);

    // Compare dual solutions
    mu_assert("Basic QP 2 test update: Error in dual solution!",
              vec_norm2_diff(work->solution->y, sols_data->y_test_new, data->m) < TESTS_TOL);


    // Compare objective values
    mu_assert("Basic QP 2 test update: Error in objective value!",
              c_absval(work->info->obj_val - sols_data->obj_value_test_new) < TESTS_TOL);

    // Clean workspace
    osqp_cleanup(work);

    // Cleanup settings and data
    c_free(settings);
    clean_problem_basic_qp2(data);
    clean_problem_basic_qp2_sols_data(sols_data);

    return 0;
}



static char * test_basic_qp2()
{

    mu_run_test(test_basic_qp2_solve);
    mu_run_test(test_basic_qp2_update);

    return 0;
}