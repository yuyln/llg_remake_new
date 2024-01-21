#define __PROFILER_IMPLEMENTATION
#include "atomistic_simulation.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define steps 100

void run_gsa(grid *g, gpu_cl *gpu) {
    double ratio = (double)g->gi.cols / g->gi.rows;
    window_init("GSA", 800 * ratio, 800);

    grid_renderer gr = grid_renderer_init(g, gpu);
    gsa_context ctx = gsa_context_init(g, gr.gpu, .T0 = 10.0, .inner_steps=700000, .print_factor=10);

    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    int state = 'h';

    double time_for_print = 1.0;
    double stopwatch_print = -1.0;
    int frames = 0;

    while(!window_should_close()) {
        switch (state) {
            case 'q':
                grid_renderer_charge(&gr);
                break;
            case 'e':
                grid_renderer_energy(&gr, 0.0);
                break;
            case 'h':
                grid_renderer_hsl(&gr);
                break;
            case 'b':
                grid_renderer_bwr(&gr);
                break;
            default:
                grid_renderer_hsl(&gr);
        }
        if (window_key_pressed('q'))
            state = 'q';
        else if (window_key_pressed('e'))
            state = 'e';
        else if (window_key_pressed('h'))
            state = 'h';
        else if (window_key_pressed('b'))
            state = 'b';

        for (int i = 0; i < steps; ++i) {
            gsa_metropolis_step(&ctx);
            gsa_thermal_step(&ctx);
        }

        window_render();
        window_poll();

        struct timespec new_time;
        clock_gettime(CLOCK_REALTIME, &new_time);
        double dt_real = (new_time.tv_sec + new_time.tv_nsec * 1.0e-9) - (current_time.tv_sec + current_time.tv_nsec * 1.0e-9);
        current_time = new_time;
        stopwatch_print += dt_real;
        frames++;
        if (stopwatch_print >= 0) {
            printf("FPS: %d - Frame Time: %e ms\n", (int)(frames / time_for_print), time_for_print / frames / 1.0e-3);
            stopwatch_print = -1.0;
            frames = 0;
        }
    }
    gsa_context_read_minimun_grid(&ctx);
    gsa_context_close(&ctx);
    grid_renderer_close(&gr);
}

void run_integration(grid *g, gpu_cl *gpu, double dt) {
    double ratio = (double)g->gi.cols / g->gi.rows;
    window_init("Integration", 800 * ratio, 800);

    grid_renderer gr = grid_renderer_init(g, gpu);
    integrate_context ctx = integrate_context_init(g, gpu, dt);

    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    int state = 'b';

    double time_for_print = 1.0;
    double stopwatch_print = -1.0;
    int frames = 0;

    while(!window_should_close()) {
        switch (state) {
            case 'q':
                grid_renderer_charge(&gr);
                break;
            case 'e':
                grid_renderer_energy(&gr, ctx.time);
                break;
            case 'h':
                grid_renderer_hsl(&gr);
                break;
            case 'b':
                grid_renderer_bwr(&gr);
                break;
            default:
                grid_renderer_hsl(&gr);
        }
        if (window_key_pressed('q'))
            state = 'q';
        else if (window_key_pressed('e'))
            state = 'e';
        else if (window_key_pressed('h'))
            state = 'h';
        else if (window_key_pressed('b'))
            state = 'b';

        for (int i = 0; i < steps; ++i) {
            integrate_step(&ctx);
            integrate_exchange_grids(&ctx);
            ctx.time += dt;
        }

        window_render();
        window_poll();

        struct timespec new_time;
        clock_gettime(CLOCK_REALTIME, &new_time);
        double dt_real = (new_time.tv_sec + new_time.tv_nsec * 1.0e-9) - (current_time.tv_sec + current_time.tv_nsec * 1.0e-9);
        current_time = new_time;
        stopwatch_print += dt_real;
        frames++;
        if (stopwatch_print >= 0) {
            printf("FPS: %d - Frame Time: %e ms - System Steps: %d - System dt: %e - System Time: %e\n", (int)(frames / time_for_print), time_for_print / frames / 1.0e-3, steps * frames, frames * steps * dt, ctx.time);
            stopwatch_print = -1.0;
            frames = 0;
        }
    }
    integrate_context_close(&ctx);
    grid_renderer_close(&gr);
}

void run_gradient_descent(grid *g, gpu_cl *gpu, double dt) {
    double ratio = (double)g->gi.cols / g->gi.rows;
    window_init("Gradient Descent", 800 * ratio, 800);

    grid_renderer gr = grid_renderer_init(g, gpu);
    gradient_descent_context ctx = gradient_descent_context_init(g, gr.gpu, .dt=dt, .T = 500.0, .T_factor = 0.9999);

    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);
    int state = 'h';

    double time_for_print = 1.0;
    double stopwatch_print = -1.0;
    int frames = 0;

    while(!window_should_close()) {
        switch (state) {
            case 'q':
                grid_renderer_charge(&gr);
                break;
            case 'e':
                grid_renderer_energy(&gr, 0.0);
                break;
            case 'h':
                grid_renderer_hsl(&gr);
                break;
            case 'b':
                grid_renderer_bwr(&gr);
                break;
            default:
                grid_renderer_hsl(&gr);
        }
        if (window_key_pressed('q'))
            state = 'q';
        else if (window_key_pressed('e'))
            state = 'e';
        else if (window_key_pressed('h'))
            state = 'h';
        else if (window_key_pressed('b'))
            state = 'b';

        for (int i = 0; i < steps; ++i) {
            gradient_descent_step(&ctx);
            gradient_descent_exchange(&ctx);
        }

        window_render();
        window_poll();

        struct timespec new_time;
        clock_gettime(CLOCK_REALTIME, &new_time);
        double dt_real = (new_time.tv_sec + new_time.tv_nsec * 1.0e-9) - (current_time.tv_sec + current_time.tv_nsec * 1.0e-9);
        current_time = new_time;
        stopwatch_print += dt_real;
        frames++;
        if (stopwatch_print >= 0) {
            printf("FPS: %d - Frame Time: %e ms - System Steps: %d - System dt: %e - System Temperature %e - System Mininum Energy: %e\n", (int)(frames / time_for_print), time_for_print / frames / 1.0e-3, steps * frames, frames * steps * dt, ctx.T, ctx.min_energy);
            stopwatch_print = -1.0;
            frames = 0;
        }
    }
    gradient_descent_read_mininum_grid(&ctx);
    gradient_descent_close(&ctx);
    grid_renderer_close(&gr);
}

//@TODO: Do 3D
//@TODO: Clear everything on integrate context and gsa context(done?)
//@TODO: Proper error handling
//@TODO: My create buffer should not be a function from gpu_cl. This difficults figuring where the error came from
int main(void) {
    grid g = {0};
    //g = grid_init(rows, cols);

    if (!grid_from_file(sv_from_cstr("./grid.grid"), &g)) {
        logging_log(LOG_WARNING, "Could not read ./grid.grid, falling back do defaults");
        int rows = 64;
        int cols = 64;
        g = grid_init(rows, cols);
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c)
                g.m[r * cols + c] = v3d_c(0.0, 0.0, 1.0);

        double J = 1.0e-3 * QE;
        grid_set_exchange(&g, J);
        grid_set_dm(&g, 0.5 * J, 0.0, R_ij);
        grid_set_alpha(&g, 0.3);
        grid_set_anisotropy(&g, (anisotropy){.ani = 0.00 * J, .dir = v3d_c(0.0, 0.0, 1.0)});
        grid_set_mu(&g, HBAR * g.gp->gamma);
        logging_log(LOG_INFO, "Gamma: %e", g.gp->gamma);
        grid_set_lattice(&g, 0.5e-9);
        g.gi.pbc.pbc_x = true;
        g.gi.pbc.pbc_y = true;

        v3d_create_skyrmion(g.m, g.gi.rows, g.gi.cols, 15, rows / 2.0, cols / 2.0, -1.0, 1.0, 0.0);
        v3d_create_skyrmion(g.m, g.gi.rows, g.gi.cols, 10, rows / 2.0, cols / 2.0, 1.0, 1.0, 0.0);
    }
    double dt = 0.01 * HBAR / (g.gp->exchange);
#if 0
    int n_stripes = 4;
    for (int i = 1; i <= n_stripes; ++i) {
        double a;
        if (i % 2 == 1) a = 0.02 * QE * 1.0e-3;
        if (i % 2 == 0) a = 0.05 * QE * 1.0e-3;
        int stripe_size = cols / n_stripes;

        int start = (i - 1) * stripe_size;
        int end = i * stripe_size;

        for (int r = 0; r < g.gi.rows; ++r)
            for (int c = start; c < end; ++c)
                grid_set_anisotropy_loc(&g, r, c, (anisotropy){.ani = a, .dir = v3d_c(0.0, 0.0, 1.0)});

    }
#endif


    string_view current_func = sv_from_cstr("current ret = (current){};\n"\
                                            "//return ret;\n"\
                                            "ret.type = CUR_STT;\n"\
                                            "ret.stt.j = v3d_c(5.0e10, 0.0, 0.0);\n"\
                                            "ret.stt.beta = 0.0;\n"\
                                            "ret.stt.polarization = -1.0;\n"\
                                            "return ret;");

    /*string_view field_func = sv_from_cstr("double hz = 0.015;\n"\
                                          "double factor = gs.exchange / gs.mu;\n"\
                                          "double w = 16.4e9 * 2.0 * M_PI;\n"\
                                          "double osc = 0.003 * (sin(w * time) + sin(2.0 * w * time));\n"\
                                          "return v3d_c(osc * factor, 0.0, hz * factor);");*/
    
    string_view field_func = sv_from_cstr("double Hz = 0.5 * gs.dm * gs.dm / 1.0 / gs.mu;\n"\
                                          "double Hy = 0.004 * gs.exchange / gs.mu;\n"\
                                          "double w = 0.017 * gs.exchange / HBAR;\n"\
                                          "double h = 2.0e-4 * sin(w * time) * gs.exchange / gs.mu;\n"\
                                          "//if (gs.col == 0 && gs.row == 0) printf(\"Hz=%e Hy=%e w=%e h=%e f=%e\\n\", Hz, Hy, w, h, w / (2.0 * M_PI));\n"\
                                          "return v3d_c(0.0, 0.0, Hz);");

    /*string_view field_func = sv_from_cstr("double H = 0.5 * gs.dm * gs.dm / gs.exchange / gs.mu; //50e-3;\n"\
                                          "double w = 8.0e9 * 2.0 * M_PI;\n"\
                                          "double theta = M_PI / 4.0;\n"\
                                          "double osc = sin(w * time);\n"\
                                          "return v3d_c(0.0, 0.0, H);");*/

    string_view temperature_func = sv_from_cstr("return 0.0;");

    string_view compile = sv_from_cstr("-cl-fast-relaxed-math");

    //integrate(&g, .dt = dt, .duration = 6.0e4 * HBAR / g.gp->exchange, .current_generation_function = current_func, .field_generation_function = field_func, .compile_augment = compile, .interval_for_information=500);

    srand(time(NULL));

    gpu_cl gpu = gpu_cl_init(current_func, field_func, temperature_func, sv_from_cstr(""), compile);
    logging_log(LOG_INFO, "Integration dt: %e", dt);
    //run_gsa(&g, &gpu);
    run_gradient_descent(&g, &gpu, 1.0e-1);
    run_integration(&g, &gpu, dt);

    //FILE *f = fopen("./grid.grid", "wb");
    //grid_dump(f, &g);
    //fclose(f);
    grid_free(&g);
    return 0;
}
