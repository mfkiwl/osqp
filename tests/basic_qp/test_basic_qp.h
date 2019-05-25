#include "osqp.h"    // OSQP API
#include "cs.h"      // CSC data structure
#include "util.h"    // Utilities for testing
#include "minunit.h" // Basic testing script header

#include "basic_qp/data.h"


static const char* test_basic_qp_solve()
{
  c_int exitflag, tmp;
  csc *mat_tmp, *P_tmp;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();

  // Define Solver settings as default
  osqp_set_default_settings(settings);
  settings->max_iter   = 2000;
  settings->alpha      = 1.6;
  settings->polish     = 1;
  settings->scaling    = 0;
  settings->verbose    = 1;
  settings->warm_start = 0;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test solve: Setup error!", exitflag == 0);


  // Solve Problem
  osqp_solve(work);

  // Compare solver statuses
  mu_assert("Basic QP test solve: Error in solver status!",
	    work->info->status_val == sols_data->status_test);

  // Compare primal solutions
  mu_assert("Basic QP test solve: Error in primal solution!",
	    vec_norm_inf_diff(work->solution->x, sols_data->x_test,
			      data->n) < TESTS_TOL);

  // Compare dual solutions
  mu_assert("Basic QP test solve: Error in dual solution!",
	    vec_norm_inf_diff(work->solution->y, sols_data->y_test,
			      data->m) < TESTS_TOL);


  // Compare objective values
  mu_assert("Basic QP test solve: Error in objective value!",
	    c_absval(work->info->obj_val - sols_data->obj_value_test) <
	    TESTS_TOL);

  // Try to set wrong settings
  mu_assert("Basic QP test solve: Wrong value of rho not caught!",
	    osqp_update_rho(work, -0.1) == 1);

  mu_assert("Basic QP test solve: Wrong value of max_iter not caught!",
	    osqp_update_max_iter(work, -1) == 1);

  mu_assert("Basic QP test solve: Wrong value of eps_abs not caught!",
	    osqp_update_eps_abs(work, -1.) == 1);

  mu_assert("Basic QP test solve: Wrong value of eps_rel not caught!",
	    osqp_update_eps_rel(work, -1.) == 1);

  mu_assert("Basic QP test solve: Wrong value of eps_prim_inf not caught!",
	    osqp_update_eps_prim_inf(work, -0.1) == 1);

  mu_assert("Basic QP test solve: Wrong value of eps_dual_inf not caught!",
	    osqp_update_eps_dual_inf(work, -0.1) == 1);

  mu_assert("Basic QP test solve: Wrong value of alpha not caught!",
	    osqp_update_alpha(work, 2.0) == 1);

  mu_assert("Basic QP test solve: Wrong value of warm_start not caught!",
	    osqp_update_warm_start(work, -1) == 1);

  mu_assert("Basic QP test solve: Wrong value of scaled_termination not caught!",
	    osqp_update_scaled_termination(work, 2) == 1);

  mu_assert("Basic QP test solve: Wrong value of check_termination not caught!",
	    osqp_update_check_termination(work, -1) == 1);

  mu_assert("Basic QP test solve: Wrong value of delta not caught!",
	    osqp_update_delta(work, 0.) == 1);

  mu_assert("Basic QP test solve: Wrong value of polish not caught!",
	    osqp_update_polish(work, 2) == 1);

  mu_assert("Basic QP test solve: Wrong value of polish_refine_iter not caught!",
	    osqp_update_polish_refine_iter(work, -1) == 1);

  mu_assert("Basic QP test solve: Wrong value of verbose not caught!",
	    osqp_update_verbose(work, 2) == 1);

  // Clean workspace
  osqp_cleanup(work);

  // Setup workspace with empty data
  exitflag = osqp_setup(&work, OSQP_NULL, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to empty data",
            exitflag == OSQP_DATA_VALIDATION_ERROR);

  // Setup workspace with empty settings
  exitflag = osqp_setup(&work, data, OSQP_NULL);
  mu_assert("Basic QP test solve: Setup should result in error due to empty settings",
            exitflag == OSQP_SETTINGS_VALIDATION_ERROR);

  // Setup workspace with wrong data->m
  tmp = data->m;
  data->m = data->m - 1;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to wrong data->m",
            exitflag == OSQP_DATA_VALIDATION_ERROR);
  data->m = tmp;

  // Setup workspace with wrong data->n
  tmp = data->n;
  data->n = data->n + 1;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to wrong data->n",
            exitflag == OSQP_DATA_VALIDATION_ERROR);

  // Setup workspace with zero data->n
  data->n = 0;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to zero data->n",
            exitflag == OSQP_DATA_VALIDATION_ERROR);
  data->n = tmp;

  // Setup workspace with wrong P->m
  tmp = data->P->m;
  data->P->m = data->n + 1;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to wrong P->n",
            exitflag == OSQP_DATA_VALIDATION_ERROR);
  data->P->m = tmp;

  // Setup workspace with non-upper-triangular P
  mat_tmp = data->P;

  // Construct non-upper-triangular P
  P_tmp = (csc*) c_malloc(sizeof(csc));
  P_tmp->m = 2;
  P_tmp->n = 2;
  P_tmp->nz = -1;
  P_tmp->nzmax = 4;
  P_tmp->x = (c_float*) c_malloc(4 * sizeof(c_float));
  P_tmp->x[0] = 4.0;
  P_tmp->x[1] = 1.0;
  P_tmp->x[2] = 1.0;
  P_tmp->x[3] = 2.0;
  P_tmp->i = (c_int*) c_malloc(4 * sizeof(c_int));
  P_tmp->i[0] = 0;
  P_tmp->i[1] = 1;
  P_tmp->i[2] = 0;
  P_tmp->i[3] = 1;
  P_tmp->p = (c_int*) c_malloc((2 + 1) * sizeof(c_int));
  P_tmp->p[0] = 0;
  P_tmp->p[1] = 2;
  P_tmp->p[2] = 4;

  data->P = P_tmp;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to non-triu structure of P",
            exitflag == OSQP_DATA_VALIDATION_ERROR);
  data->P = mat_tmp;

  // Setup workspace with non-consistent bounds
  data->l[0] = data->u[0] + 1.0;
  exitflag = osqp_setup(&work, data, settings);
  mu_assert("Basic QP test solve: Setup should result in error due to non-consistent bounds",
            exitflag == OSQP_DATA_VALIDATION_ERROR);


  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);
  c_free(P_tmp->x);
  c_free(P_tmp->i);
  c_free(P_tmp->p);
  c_free(P_tmp);

  return 0;
}

#ifdef ENABLE_MKL_PARDISO
static char* test_basic_qp_solve_pardiso()
{
  c_int exitflag;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();


  // Define Solver settings as default
  osqp_set_default_settings(settings);
  settings->max_iter      = 2000;
  settings->alpha         = 1.6;
  settings->polish        = 1;
  settings->scaling       = 0;
  settings->verbose       = 1;
  settings->warm_start    = 0;
  settings->linsys_solver = MKL_PARDISO_SOLVER;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test solve Pardiso: Setup error!", exitflag == 0);

  // Solve Problem
  osqp_solve(work);

  // Compare solver statuses
  mu_assert("Basic QP test solve Pardiso: Error in solver status!",
            work->info->status_val == sols_data->status_test);

  // Compare primal solutions
  mu_assert("Basic QP test solve Pardiso: Error in primal solution!",
            vec_norm_inf_diff(work->solution->x, sols_data->x_test,
                              data->n) < TESTS_TOL);

  // Compare dual solutions
  mu_assert("Basic QP test solve Pardiso: Error in dual solution!",
            vec_norm_inf_diff(work->solution->y, sols_data->y_test,
                              data->m) < TESTS_TOL);


  // Compare objective values
  mu_assert("Basic QP test solve Pardiso: Error in objective value!",
            c_absval(work->info->obj_val - sols_data->obj_value_test) <
            TESTS_TOL);

  // Clean workspace
  osqp_cleanup(work);


  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}
#endif

static const char* test_basic_qp_update()
{
  c_int exitflag;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();


  // Define Solver settings as default
  osqp_set_default_settings(settings);
  settings->max_iter   = 200;
  settings->alpha      = 1.6;
  settings->polish     = 1;
  settings->scaling    = 0;
  settings->verbose    = 1;
  settings->warm_start = 0;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test update: Setup error!", exitflag == 0);


  // ====================================================================
  //  Update data
  // ====================================================================

  // Update linear cost
  osqp_update_lin_cost(work, sols_data->q_new);
  mu_assert("Basic QP test update: Error in updating linear cost!",
            vec_norm_inf_diff(work->data->q, sols_data->q_new,
                              data->n) < TESTS_TOL);

  // UPDATE BOUND
  // Try to update with non-consistent values
  mu_assert("Basic QP test update: Error in bounds update ordering not caught!",
            osqp_update_bounds(work, sols_data->u_new, sols_data->l_new) == 1);

  // Now update with correct values
  mu_assert("Basic QP test update: Error in bounds update ordering!",
            osqp_update_bounds(work, sols_data->l_new, sols_data->u_new) == 0);

  mu_assert("Basic QP test update: Error in bounds update, lower bound!",
            vec_norm_inf_diff(work->data->l, sols_data->l_new,
                              data->m) < TESTS_TOL);

  mu_assert("Basic QP test update: Error in bounds update, upper bound!",
            vec_norm_inf_diff(work->data->u, sols_data->u_new,
                              data->m) < TESTS_TOL);

  // Return original values
  osqp_update_bounds(work, data->l, data->u);


  // UPDATE LOWER BOUND
  // Try to update with non-consistent values
  mu_assert(
    "Basic QP test update: Error in lower bound update ordering not caught!",
    osqp_update_lower_bound(work, sols_data->u_new) == 1);

  // Now update with correct values
  mu_assert("Basic QP test update: Error in lower bound update ordering!",
            osqp_update_lower_bound(work, sols_data->l_new) == 0);

  mu_assert("Basic QP test update: Error in updating lower bound!",
            vec_norm_inf_diff(work->data->l, sols_data->l_new,
                              data->m) < TESTS_TOL);

  // Return original values
  osqp_update_lower_bound(work, data->l);


  // UPDATE UPPER BOUND
  // Try to update with non-consistent values
  mu_assert(
    "Basic QP test update: Error in upper bound update: ordering not caught!",
    osqp_update_upper_bound(work, sols_data->l_new) == 1);

  // Now update with correct values
  mu_assert("Basic QP test update: Error in upper bound update: ordering!",
            osqp_update_upper_bound(work, sols_data->u_new) == 0);

  mu_assert("Basic QP test update: Error in updating upper bound!",
            vec_norm_inf_diff(work->data->u, sols_data->u_new,
                              data->m) < TESTS_TOL);


  // Clean workspace
  osqp_cleanup(work);


  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}

static const char* test_basic_qp_check_termination()
{
  c_int exitflag;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();


  // Define Solver settings as default
  osqp_set_default_settings(settings);
  settings->max_iter          = 200;
  settings->alpha             = 1.6;
  settings->polish            = 0;
  settings->scaling           = 0;
  settings->verbose           = 1;
  settings->check_termination = 0;
  settings->warm_start        = 0;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test solve: Setup error!", exitflag == 0);

  // Solve Problem
  osqp_solve(work);

  // Check if iter == max_iter
  mu_assert(
    "Basic QP test check termination: Error in number of iterations taken!",
    work->info->iter == work->settings->max_iter);

  // Compare solver statuses
  mu_assert("Basic QP test check termination: Error in solver status!",
            work->info->status_val == sols_data->status_test);

  // Compare primal solutions
  mu_assert("Basic QP test check termination: Error in primal solution!",
            vec_norm_inf_diff(work->solution->x, sols_data->x_test,
                              data->n) < TESTS_TOL);

  // Compare dual solutions
  // print_vec(work->solution->y, data->m, "y_sol");
  // print_vec(sols_data->y_test, data->m, "y_test");
  mu_assert("Basic QP test check termination: Error in dual solution!",
            vec_norm_inf_diff(work->solution->y, sols_data->y_test,
                              data->m) < TESTS_TOL);

  // Compare objective values
  mu_assert("Basic QP test check termination: Error in objective value!",
            c_absval(work->info->obj_val - sols_data->obj_value_test) <
            TESTS_TOL);

  // Clean workspace
  osqp_cleanup(work);

  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}

static const char* test_basic_qp_update_rho()
{
  c_int extiflag;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Exitflag
  c_int exitflag;

  // rho to use
  c_float rho;

  // Define number of iterations to compare
  c_int n_iter_new_solver, n_iter_update_rho;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();


  // Define Solver settings as default
  rho = 0.7;
  osqp_set_default_settings(settings);
  settings->rho               = rho;
  settings->adaptive_rho      = 0; // Disable adaptive rho for this test
  settings->eps_abs           = 5e-05;
  settings->eps_rel           = 5e-05;
  settings->check_termination = 1;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test update rho: Setup error!", exitflag == 0);

  // Solve Problem
  osqp_solve(work);

  // Store number of iterations
  n_iter_new_solver = work->info->iter;

  // Compare solver statuses
  mu_assert("Update rho test solve: Error in solver status!",
            work->info->status_val == sols_data->status_test);

  // Compare primal solutions
  mu_assert("Update rho test solve: Error in primal solution!",
            vec_norm_inf_diff(work->solution->x, sols_data->x_test,
                              data->n)/vec_norm_inf(sols_data->x_test, data->n) < TESTS_TOL);

  // Compare dual solutions
  mu_assert("Update rho test solve: Error in dual solution!",
            vec_norm_inf_diff(work->solution->y, sols_data->y_test,
                              data->m)/vec_norm_inf(sols_data->y_test, data->m) < TESTS_TOL);

  // Compare objective values
  mu_assert("Update rho test solve: Error in objective value!",
            c_absval(work->info->obj_val - sols_data->obj_value_test) <
            TESTS_TOL);

  // Clean workspace
  osqp_cleanup(work);


  // Create new problem with different rho and update it
  osqp_set_default_settings(settings);
  settings->rho               = 0.1;
  settings->adaptive_rho      = 0;
  settings->check_termination = 1;
  settings->eps_abs           = 5e-05;
  settings->eps_rel           = 5e-05;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test update rho: Setup error!", exitflag == 0);

  // Update rho
  exitflag = osqp_update_rho(work, rho);
  mu_assert("Basic QP test update rho: Error update rho!", exitflag == 0);

  // Solve Problem
  osqp_solve(work);

  // Compare solver statuses
  mu_assert("Basic QP test update rho: Error in solver status!",
            work->info->status_val == sols_data->status_test);

  // Compare primal solutions
  mu_assert("Basic QP test update rho: Error in primal solution!",
            vec_norm_inf_diff(work->solution->x, sols_data->x_test,
                              data->n)/vec_norm_inf(sols_data->x_test, data->n) < TESTS_TOL);

  // Compare dual solutions
  mu_assert("Basic QP test update rho: Error in dual solution!",
            vec_norm_inf_diff(work->solution->y, sols_data->y_test,
                              data->m)/vec_norm_inf(sols_data->y_test, data->m)< TESTS_TOL);

  // Compare objective values
  mu_assert("Basic QP test update rho: Error in objective value!",
            c_absval(work->info->obj_val - sols_data->obj_value_test) <
            TESTS_TOL);

  // Get number of iterations
  n_iter_update_rho = work->info->iter;

  // Assert same number of iterations
  mu_assert("Basic QP test update rho: Error in number of iterations!",
            n_iter_new_solver == n_iter_update_rho);

  // Cleanup solver
  osqp_cleanup(work);

  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}

static const char* test_basic_qp_time_limit()
{
  c_int exitflag;

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();

  // Define Solver settings as default
  osqp_set_default_settings(settings);

  // Check default time limit
  mu_assert("Basic QP test time limit: Default not correct", settings->time_limit == 0);

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Setup correct
  mu_assert("Basic QP test time limit: Setup error!", exitflag == 0);

  // Solve Problem
  osqp_solve(work);

  // Compare solver statuses
  mu_assert("Basic QP test time limit: Error in no time limit solver status!",
	    work->info->status_val == sols_data->status_test);

  // Update time limit
  osqp_update_time_limit(work, 1e-5);
  osqp_update_max_iter(work, (c_int)2e9);
  osqp_update_check_termination(work, 0);

  // Solve Problem
  osqp_solve(work);

  // Compare solver statuses
  mu_assert("Basic QP test time limit: Error in timed out solver status!",
	    work->info->status_val == OSQP_TIME_LIMIT_REACHED);

  // Cleanup solver
  osqp_cleanup(work);

  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}


static const char* test_basic_qp_warm_start()
{
  c_int exitflag, iter;

  // Cold started variables
  c_float x0[2] = { 0.0, 0.0, };
  c_float y0[4] = { 0.0, 0.0, 0.0, 0.0, };

  // Optimal solution
  c_float xopt[2] = { 0.3, 0.7, };
  c_float yopt[4] = {-2.9, 0.0, 0.2, 0.0, };

  // Problem settings
  OSQPSettings *settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));

  // Structures
  OSQPWorkspace *work; // Workspace
  OSQPData *data;      // Data
  basic_qp_sols_data *sols_data;

  // Populate data
  data = generate_problem_basic_qp();
  sols_data = generate_problem_basic_qp_sols_data();

  // Define Solver settings as default
  osqp_set_default_settings(settings);
  settings->check_termination = 1;

  // Setup workspace
  exitflag = osqp_setup(&work, data, settings);

  // Solve Problem
  osqp_solve(work);
  iter = work->info->iter;

  // Cold start and solve again
  osqp_warm_start(work, x0, y0);
  osqp_solve(work);

  // Check if the number of iterations is the same
  mu_assert("Basic QP test warm start: Cold start error!", work->info->iter == iter);

  // Warm start from the solution and solve again
  osqp_warm_start_x(work, xopt);
  osqp_warm_start_y(work, yopt);
  osqp_solve(work);

  // Check that the number of iterations equals 1
  mu_assert("Basic QP test warm start: Warm start error!", work->info->iter == 1);

  // Cleanup solver
  osqp_cleanup(work);

  // Cleanup data
  clean_problem_basic_qp(data);
  clean_problem_basic_qp_sols_data(sols_data);

  // Cleanup
  c_free(settings);

  return 0;
}


static const char* test_basic_qp()
{
  mu_run_test(test_basic_qp_solve);
#ifdef ENABLE_MKL_PARDISO
  mu_run_test(test_basic_qp_solve_pardiso);
#endif
  mu_run_test(test_basic_qp_update);
  mu_run_test(test_basic_qp_check_termination);
  mu_run_test(test_basic_qp_update_rho);
  mu_run_test(test_basic_qp_time_limit);
  mu_run_test(test_basic_qp_warm_start);

  return 0;
}
