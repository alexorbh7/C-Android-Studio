
/*
 * BRICK SCENE
 *
 * alexorbh7@hotmail.com
 */


#include "Brick_Scene.hpp"
#include "Intro_Scene.hpp"
#include "Main_Menu_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>
#include <basics/Log>
#include <basics/Scaling>
#include <basics/Rotation>
#include <basics/Translation>
#include <basics/Timer>

using namespace basics;
using namespace std;

namespace example
{

    Brick_Scene::Texture_Data Brick_Scene::textures_data[] =
    {
        { ID(loading),    "game-scene/loading.png"        },

        //posible error en la carga de ids
        //el id que habia aqui debajo ya no funciona/ lo he borrado
        { ID(buttonID),       "game-scene/horizontal-bar.png"        },
        { ID(hbar),       "game-scene/horizontal-bar.png" },
        { ID(vbar),       "game-scene/vertical-bar.png"   },
        { ID(verticalbar),       "game-scene/new-vertical-bar.png"   },
        { ID(player-bar), "game-scene/players-bar.png"    },
        { ID(projectile),       "game-scene/ball.png"           },
        { ID(projectile2),       "game-scene/ball.png"           },
        { ID(brick),      "game-scene/players-bar.png"           },
        { ID(logo),       "test.png"           },
        { ID(testbutton),       "game-scene/loading.png"           },
        //for menus
        { ID(pauseID),               "pause3.png"           },
        { ID(helpID),               "help3.png"           },
        { ID(continueID),           "play2.png"           },
        { ID(exitID),               "exit2.png"           },
        { ID(closehelpID),          "close.png"           },
        { ID(helpbackgroundID),      "helpmenu.png"           },
        { ID(pausebackgroundID),      "menupausa.png"           },

    };

    unsigned Brick_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------

    //cambiar en el hpp
    constexpr float Brick_Scene::   projectile_speed;
    constexpr float Brick_Scene::   player_speed;

    // ---------------------------------------------------------------------------------------------

    //Constructor
    Brick_Scene::Brick_Scene()
    {
        //EN ESTE JUEGO, LA Y Height ES EL ANCHO DE LA PANTALLA (LO MAS CORTO)
        //LA X width ES EL ALTO (LO MÁS LARGO)
        canvas_height  = 720;
        canvas_width =  1280;

        srand (unsigned(time(nullptr)));

        initialize ();

        //  outsidePositionX = 100.f;
        //  outsidePositionY = 100.f;
    }

    // ---------------------------------------------------------------------------------------------

    //Inicializa el juego al crearse, es como el Start de Unity
    bool Brick_Scene::initialize ()
    {
        state    = LOADING;
        paused   = true;
        gameplay = UNINITIALIZED;

        game_paused = false;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::suspend ()
    {
        paused = true;
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::resume ()
    {
        paused = false;
    }

    // ---------------------------------------------------------------------------------------------

    //Manejador de eventos del usuario
    void Brick_Scene::handle (Event & event)
    {
        if (state == RUNNING)
        {
            if (gameplay == WAITING_TO_START)
            {
                start_playing ();
            }
            else switch (event.id)
            {
                case ID(touch-started):
                {
                    Shoot();
                    x = *event[ID(x)].as< var::Float > ();
                    y = *event[ID(y)].as< var::Float > ();

                    input_button_touched = true;

                    break;
                }
                case ID(touch-moved):
                {
                    user_target_x = *event[ID(y)].as< var::Float > ();
                    input_button_touched = false;
                    follow_target = true;
                    break;
                }

                case ID(touch-ended):
                {
                    input_button_touched = false;
                    follow_target = false;
                    break;
                }

            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::update (float time)
    {
        if (!paused)
            switch (state)
            {
                case LOADING: load_textures  ();
                    break;
                case RUNNING:
                    if(!game_paused)
                        run_simulation (time); break;
                case ERROR:   break;
            }

        // Aunque esté en pausa debe continuar mirando los botones
        check_touch_continue(x, y);
        check_touch_help(x, y);
        check_touch_exit(x, y);
        check_touch_pause(x, y);
        check_touch_close_help(x, y);
        // float a=pause_button->get_right_x();
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::render (basics::Graphics_Context::Accessor & context)
    {
        if (!paused)
        {
            Canvas * canvas = context->get_renderer< Canvas > (ID(canvas));

            if (!canvas)
            {
                canvas = Canvas::create (ID(canvas), context, {{ canvas_width, canvas_height }});
            }

            if (canvas)
            {
                canvas->clear ();

                switch (state)
                {
                    case LOADING: render_loading   (*canvas); break;
                    case RUNNING: render_playfield (*canvas); break;
                    case ERROR:   break;
                }
            }
        }
    }


    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::load_textures ()
    {
        Graphics_Context::Accessor context = director.lock_graphics_context ();

        if (context)
        {
            // Se carga la siguiente textura:

            Texture_Data   & texture_data = textures_data[textures.size ()];
            Texture_Handle & texture      = textures[texture_data.id] = Texture_2D::create (texture_data.id, context, texture_data.path);

            // Se comprueba si la textura se ha podido cargar correctamente:

            if (texture) context->add (texture); else state = ERROR;

            // Se han terminado de cargar todas las texturas:

            if (textures.size () == textures_count)
            {
                create_sprites ();
                restart_game   ();

                state = RUNNING;
            }
        }
    }

    void Brick_Scene::create_sprites()
    {
        //Sprites de fondo
        Sprite_Handle    top_bar(new Sprite( textures[ID(hbar)].get () ));
        Sprite_Handle middle_bar(new Sprite( textures[ID(verticalbar)].get () ));
        Sprite_Handle bottom_bar(new Sprite( textures[ID(hbar)].get () ));

        top_bar->set_anchor   (TOP | LEFT);
        top_bar->set_position ({ 0, canvas_height });
        middle_bar->set_anchor   (CENTER);
        middle_bar->set_position ({ 0 , canvas_height / 2.f });
        bottom_bar->set_anchor   (BOTTOM | LEFT);
        bottom_bar->set_position ({ 0, 0 });

        sprites.push_back (   top_bar);
        sprites.push_back (middle_bar);
        sprites.push_back (bottom_bar);

        //Sprites de juego
        Sprite_Handle  player_handle(new Sprite( textures[ID(player-bar)].get () ));
        Sprite_Handle  projectile_handle(new Sprite( textures[ID(projectile)  ].get () ));

        sprites.push_back(player_handle);
        sprites.push_back(projectile_handle);

        //Punteros para los sprites que usemos con frecuencia
        top_border  =  middle_bar.get ();
        left_border =  bottom_bar.get ();
        right_border=  top_bar.get ();
        player  =      player_handle.get ();
        projectile  =  projectile_handle.get ();

        /*
        //button
        Sprite_Handle  projectile2_handle(new Sprite( textures[ID(logo)  ].get () ));
        sprites.push_back(projectile2_handle);
        projectile2  =  projectile2_handle.get ();
        projectile2->set_position ({0 ,0});
        */

        //Creating bricks array
        int x, y, k=1;
        for (int i = 1; i < 4; ++i)
        {
            for (int j = 1; j < 3; ++j)
            {
                Sprite_Handle    brick_handle(new Sprite( textures[ID(player-bar)].get () ));
                brick_handle->set_position ({100 + i*100 , 20 + j * 250});
                sprites.push_back (   brick_handle);
                bricks[k] = brick_handle.get ();
               // bricks[k]->hide();
                k++;
            }
        }

        //creating pause and help buttons/ menus
        Sprite_Handle pause(new Sprite( textures[ID(pauseID)].get () ));
        Sprite_Handle continue_b(new Sprite( textures[ID(continueID)].get () ));
        Sprite_Handle exit_b(new Sprite( textures[ID(exitID)].get () ));
        Sprite_Handle pause_background(new Sprite( textures[ID(pausebackgroundID)].get () ));
        Sprite_Handle help_bg(new Sprite( textures[ID(helpbackgroundID)].get () ));
        Sprite_Handle help_bt(new Sprite( textures[ID(helpID)].get () ));
        Sprite_Handle close_help(new Sprite( textures[ID(closehelpID)].get () ));

        //pause button
        pause->set_anchor   (CENTER);
        pause->set_position ({ 50, 50 });

        //pause menu
        pause_background->set_anchor   (CENTER);
        pause_background->set_position ({ canvas_width / 2.f, canvas_height / 2.f});
        continue_b->set_anchor   (CENTER);
        continue_b->set_position ({ canvas_width / 3.f * 2, canvas_height / 3.f });
        exit_b->set_anchor   (CENTER);
        exit_b->set_position ({ canvas_width / 3.f * 2, canvas_height / 3.f  * 2});


        //help button
        help_bt->set_anchor   (CENTER);
        help_bt->set_position ({ 50, canvas_height - 50.f});

        //help menu
        help_bg->set_anchor   (CENTER);
        help_bg->set_position ({ canvas_width / 2.f, canvas_height / 2.f});
        close_help->set_anchor   (CENTER);
        close_help->set_position ({ canvas_width / 3.f * 2 -90, canvas_height / 3.f });

        sprites.push_back (pause);
        sprites.push_back (continue_b);
        sprites.push_back (exit_b);
        sprites.push_back (pause_background);
        sprites.push_back (help_bg);
        sprites.push_back (help_bt);
        sprites.push_back (close_help);


        pause_button =  pause.get ();
        continue_button=  continue_b.get ();
        exit_button=  exit_b.get ();
        pause_menu_background=  pause_background.get ();
        help_background=  help_bg.get ();
        help_button=  help_bt.get ();
        close_help_button=  close_help.get ();

        //pause_button -> hide();
        continue_button -> hide();
        exit_button -> hide();
        pause_menu_background -> hide();
        help_background -> hide();
        //help_button -> hide();
        close_help_button -> hide();


    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::restart_game()
    {
        //recolocar jugador
        player->set_position ({canvas_width - player->get_width () * 3.f, canvas_height / 2.f });
        player->set_speed_y  (0.f);

        //recolocar proyectil
        projectile->set_position ({canvas_width - player->get_width () * 5.f , canvas_height / 2.f });
        projectile->set_speed_x  (0.f);

        //recolocar bricks
        //brick=bricks[1];
        //brick->set_position ({brick->get_width () * 3.f, canvas_height / 2.f });

        projectileShoot = false;
        follow_target = false;
        //destroyedBrick = false;
        for (int i=0;i<6;i++)
        {
            brick=bricks[i+1];
            brick->show();
            //brick->set_position ({300, canvas_height / 2.f });
            destroyedBrickArray[i]=false;
        }

        enable_pause_button = true;
        enable_help_button = true;
        enable_close_help_button = false;
        enable_continue_button = false;
        enable_exit_button = false;

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::start_playing ()
    {
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        gameplay = PLAYING;
        input_button_touched = false;
        //brick->set_speed_y  (-1.f);

        projectileShoot = false;
        //destroyedBrick = false;
        int k=0;
        for (int i=0;i<7;i++)
        {

            destroyedBrickArray[k]=false;
            enableBrickCollisionsArray[k]= true;
            k++;
        }

        enable_pause_button = true;
        enable_help_button = true;
        enable_close_help_button = false;
        enable_continue_button = false;
        enable_exit_button = false;

    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        //update_bricks   ();
        update_user ();

        //Added
        update_projectile();

        // Se comprueban las posibles colisiones de la bola con los bordes y con los players:

        check_collisions ();
        check_bricks_collisions();

        check_touch_continue(x, y);
        check_touch_help(x, y);
        check_touch_exit(x, y);
        check_touch_pause(x, y);
        check_touch_close_help(x, y);


    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::update_user ()
    {
        if (player->intersects (*left_border))
        {
            player->set_position_y (left_border->get_top_y () + player->get_height () / 2.f);
            player->set_speed_y (0);
        }
        else
        if (player->intersects (*right_border))
        {
            player->set_position_y (right_border->get_bottom_y () - player->get_height () / 2.f);
            player->set_speed_y (0);
        }
        else
        if (follow_target)
        {
            float delta_y = user_target_x - player->get_position_y ();

            if (delta_y < 0.f) player->set_speed_y (-player_speed); else
            if (delta_y > 0.f) player->set_speed_y (+player_speed);
        }
        else
            player->set_speed_y (0);
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Updates the position of the proj depending on the player position
     */
    void Brick_Scene::update_projectile ()
    {
        if(!projectileShoot)
        {
            projectile->set_speed_y (player->get_speed_y());
            projectile->set_position_y (player->get_position_y());
        }
    }

    // --------------------------------------------------------------------------------------------

    void Brick_Scene::check_collisions ()
    {
        if(gameplay != BALL_LEAVING) {
            if (projectile->intersects(*left_border)) {
                //brick->set_position_y (top_border->get_bottom_y () - brick->get_height () / 2.f);
                projectile->set_speed_y(-projectile->get_speed_y());
            } else if (projectile->intersects(*right_border)) {
                //brick->set_position_y (right_border->get_top_y () + brick->get_height () / 2.f);
                projectile->set_speed_y(-projectile->get_speed_y());
            } else if (projectile->intersects(*top_border)) {
                //brick->set_position_y (right_border->get_top_y () + brick->get_height () / 2.f);
                projectile->set_speed_x(-projectile->get_speed_x());

                //enableBrickCollisions = true;
                for (int i=0;i<7;++i)
                enableBrickCollisionsArray[i]= true;
            }

            //colision con jugador/player
            if (projectile->get_right_x() > player->get_left_x())  //collision proyectil - Player
            {
                if (projectile->get_top_y() > player->get_bottom_y() &&
                    projectile->get_bottom_y() < player->get_top_y()) {
                    projectile->set_position_x(
                            player->get_left_x() - projectile->get_width() / 2.f);
                    projectile->set_speed_x(-projectile->get_speed_x());
                    //projectile->set_speed_y(-projectile->get_speed_x()*/);
                    enableBrickCollisions = true;

                    for (int i=0;i<7;++i)
                        enableBrickCollisionsArray[i]= true;
                } else    //si no choca con nosotros por debajo, hemos perdido
                    gameplay = BALL_LEAVING;
            }
        }
        else
        if ( projectile->get_left_x () > float(canvas_width))
        {
            restart_game ();
        }

    }

    // --------------------------------------------------------------------------------------------

    /*
     * Checks the brick collisiones with the projectile
     */
    void Brick_Scene::check_bricks_collisions()
    {
        if (projectileShoot) // && enableBrickCollision

        {
            int k=1;
            for (int i = 0; i < 6; i++)
            {

                 brick = bricks[k];
         //      brick->hide();
  //              projectile2->set_position ({100+k*50 , 600 });
  //              projectile2->set_speed_x  (0.f);

                if (enableBrickCollisionsArray[k] /*&& !destroyedBrickArray[k]*/) //Wall collisions
                {
                    //keep a copy reference to check if the speed goes right or left
                    float projectile_speed_x = projectile->get_speed_x();
                    float projectile_speed_y = projectile->get_speed_y();
                    //we need to check of the positions of the projectile in x/y are between
                    //the values of each brick
                    if ((projectile->get_right_x() > brick->get_left_x() && projectile->get_speed_x() > 0
                         && projectile->get_right_x() < brick->get_right_x())
                         || (projectile->get_left_x() < brick->get_right_x() && projectile->get_speed_x() < 0
                         && projectile->get_right_x() > brick->get_left_x()))
                    {
                        if(!destroyedBrickArray[k])
                        {
                            //comprobación de colisión con stick player
                            //enableBrickCollisions = false;   //if we dont set this, the firrst if is true and keeps asking for the second condition
                            //en cuanto lo rompemos, lo ponemos a false

                            enableBrickCollisionsArray[k]= true;
                            if (projectile->get_bottom_y() < brick->get_top_y()
                                && projectile->get_top_y() > brick->get_bottom_y()  )
                            {

                                projectile->set_speed_x(-projectile->get_speed_x());
                                brick->hide();

                                i = 7;  //nos salimos del bucle para ahorrar proceso


                                //si viene por debajo
                                if (projectile_speed_x > 0)
                                    projectile->set_position_x(
                                            brick->get_left_x() - projectile->get_width());
                                else
                                    projectile->set_position_x(
                                            brick->get_right_x() + projectile->get_width());

                                //projectile->set_speed_x(-projectile->get_speed_x());
                                //destroyedBrick = true;
                                destroyedBrickArray[k]= true;
                                for (int m = 0; m < 7; ++m) //los ponemos todos a false para que no se pueda romper otra vez ninguno
                                    enableBrickCollisionsArray[m]= true;
                                bool checknoBricks = true;
                                for (int m = 1; m < 7; ++m) //true && false = false, no estan todos destruidos
                                    checknoBricks= checknoBricks&&destroyedBrickArray[m];

                                //si todos han sido destruidos, el bool del array se mantiene a true
                                if (checknoBricks)
                                    gameplay = BALL_LEAVING;

                            }
                        }

                    }
                }
            k++;
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::render_loading (Canvas & canvas)
    {
        Texture_2D * loading_texture = textures[ID(loading)].get ();

        if (loading_texture)
        {
            canvas.fill_rectangle
            (
                    { canvas_width * .5f, canvas_height * .5f },
                    { loading_texture->get_width (), loading_texture->get_height () },
                    loading_texture
            );

        }
    }

    // ---------------------------------------------------------------------------------------------

    void Brick_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    //--------------------------------------------------------------------------------------------

    /*
     * Shoot a projectile
     * The rand's seed has been initialized in the initialize function
     */
    void Brick_Scene::Shoot()
    {
        if(!projectileShoot)
        {
            Vector2f random_direction
                    (
                            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
                            float(rand () % int(canvas_height) - int(canvas_height / 2))
                    );

            projectile->set_speed (random_direction.normalized () * projectile_speed);
            //projectile->set_speed_x(-5.f);
            //projectile->set_speed_y(-10.f);
            projectileShoot = true;
        }
    }

    // ---------------------------------------------------------------------------------------------


    /*
     * checks if we have pressed the pause button, freezes game
     * activates the bools and sprites for the continue and exit buttons
     */
    void Brick_Scene::check_touch_pause(float x, float y)
    {
        if(enable_pause_button)
        {
            float xMin, xMax, yMin, yMax;

            xMin = pause_button->get_position_x() - (pause_button->get_height() / 2);
            xMax = pause_button->get_position_x() + (pause_button->get_height() / 2);
            yMin = pause_button->get_position_y() - (pause_button->get_width() / 2);
            yMax = pause_button->get_position_y() + (pause_button->get_width() / 2);

            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                game_paused = true;
                pause_menu_background->show();
                enable_continue_button = true;
                continue_button->show();
                enable_exit_button = true;
                exit_button->show();
                input_button_touched = false;
                enable_help_button = false;

            }
        }

    }

    //checks if we hacve pressed the button exit, loads the main menu scene
    void Brick_Scene::check_touch_exit(float x, float y)
    {
       if(enable_exit_button)
        {
            float xMin=0, xMax=0, yMin=0, yMax=0;
            //xMin = pause_button->get_position_x();
            xMin = exit_button->get_position_x() - (exit_button->get_height() / 2);
            xMax = exit_button->get_position_x() + (exit_button->get_height() / 2);
            yMin = exit_button->get_position_y() - (exit_button->get_width() / 2);
            yMax = exit_button->get_position_y() + (exit_button->get_width() / 2);
            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                director.run_scene(shared_ptr< Scene >(new Main_Menu_Scene));
            }
        }

    }


    void Brick_Scene::check_touch_continue(float x, float y)
    {
        if(enable_continue_button)
        {
            float xMin, xMax, yMin, yMax;
            xMin = continue_button->get_position_x() - (continue_button->get_height() / 2);
            xMax = continue_button->get_position_x() + (continue_button->get_height() / 2);
            yMin = continue_button->get_position_y() - (continue_button->get_width() / 2);
            yMax = continue_button->get_position_y() + (continue_button->get_width() / 2);
            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                game_paused = false;
                pause_menu_background->hide();
                enable_continue_button = false;
                continue_button->hide();
                enable_exit_button = false;
                exit_button->hide();
                enable_help_button = true;
                input_button_touched = false;

            }
        }

    }

    void Brick_Scene::check_touch_help(float x, float y)
    {
        if(enable_help_button)
        {
            float xMin, xMax, yMin, yMax;
            xMin = help_button->get_position_x() - (help_button->get_height() / 2);
            xMax = help_button->get_position_x() + (help_button->get_height() / 2);
            yMin = help_button->get_position_y() - (help_button->get_width() / 2);
            yMax = help_button->get_position_y() + (help_button->get_width() / 2);
            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                game_paused = true;
                help_background->show();
                help_button->hide();
                enable_help_button = false;
                close_help_button->show();
                enable_pause_button = false;

                enable_close_help_button = true;
                input_button_touched = false;
            }
        }

    }

    void Brick_Scene::check_touch_close_help(float x, float y)
    {
        if(enable_close_help_button)
        {
            float xMin, xMax, yMin, yMax;
            xMin = close_help_button->get_position_x() - (close_help_button->get_height() / 2);
            xMax = close_help_button->get_position_x() + (close_help_button->get_height() / 2);
            yMin = close_help_button->get_position_y() - (close_help_button->get_width() / 2);
            yMax = close_help_button->get_position_y() + (close_help_button->get_width() / 2);
            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                game_paused = false;
                help_background->hide();
                close_help_button -> hide();
                pause_button-> show();
                help_button -> show();
                enable_help_button = true;
                enable_pause_button = true;
                enable_close_help_button = false;
                input_button_touched = false;
            }
        }

    }


}
