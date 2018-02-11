
/*
 * SPACE SCENE
 *
 * alexorbh7@hotmail.com
 */

#include "Brick_Scene.hpp"
#include "Intro_Scene.hpp"
#include "Space_Scene.hpp"

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

    Space_Scene::Texture_Data Space_Scene::textures_data[] =
    {
        { ID(loading),    "game-scene/loading.png"        },
        { ID(hbar),       "game-scene/horizontal-bar.png" },
        { ID(vbar),       "game-scene/vertical-bar.png"   },
        { ID(verticalbar),       "game-scene/new-vertical-bar.png"   },
        { ID(player-bar), "game-scene/players-bar.png"    },
        { ID(projectile),       "missile.png"           },
        { ID(button),       "game-scene/ball.png"           },
        { ID(shoot),        "play2.png"           },
        { ID(ship),        "ship.png"           },
        { ID(alien),        "alien.png"           },
        { ID(logo),         "test.png"           },
    };

    unsigned Space_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------

    //cambiar en el hpp
    constexpr float Space_Scene::   projectile_speed;
    constexpr float Space_Scene::   player_speed;
    constexpr float Space_Scene::   alien_speed;

    // ---------------------------------------------------------------------------------------------

    //Constructor
    Space_Scene::Space_Scene()
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
    bool Space_Scene::initialize ()
    {
        state    = LOADING;
        paused   = true;
        gameplay = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::suspend ()
    {
        paused = true;
    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::resume ()
    {
        paused = false;
    }

    // ---------------------------------------------------------------------------------------------

    //Manejador de eventos del usuario
    void Space_Scene::handle (Event & event)
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

    void Space_Scene::update (float time)
    {
        if (!paused)
            switch (state)
            {
                case LOADING: load_textures  ();     break;
                case RUNNING: run_simulation (time);
                    break;
                case ERROR:   break;
            }
    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::render (basics::Graphics_Context::Accessor & context)
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

    void Space_Scene::load_textures ()
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

    void Space_Scene::create_sprites()
    {
        //Sprites de fondo
        Sprite_Handle    top_bar(new Sprite( textures[ID(hbar)].get () ));
        Sprite_Handle middle_bar(new Sprite( textures[ID(verticalbar)].get () ));
        //Sprite_Handle right_bar (new Sprite( textures[ID(verticalbar)].get () ));
        Sprite_Handle bottom_bar(new Sprite( textures[ID(hbar)].get () ));

        top_bar->set_anchor   (TOP | LEFT);
        top_bar->set_position ({ 0, canvas_height });
        middle_bar->set_anchor   (CENTER);
        middle_bar->set_position ({ 0.f , canvas_height / 2.f });
        //right_bar->set_anchor   (CENTER);
        //right_bar->set_position ({ 100 , canvas_height / 2.f });
        bottom_bar->set_anchor   (BOTTOM | LEFT);
        bottom_bar->set_position ({ 0, 0 });

        sprites.push_back (   top_bar);
        sprites.push_back (middle_bar);
        sprites.push_back (bottom_bar);

        //Sprites de juego
        Sprite_Handle  player_handle(new Sprite( textures[ID(ship)].get () ));

        //   Sprite_Handle  brick_handle(new Sprite( textures[ID(player-bar)].get () ));

        sprites.push_back(player_handle);
        //  sprites.push_back(brick_handle);

        //Punteros para los sprites que usemos con frecuencia
        top_border  =  middle_bar.get ();
        left_border =  bottom_bar.get ();
        right_border=  top_bar.get ();
        player  =      player_handle.get ();
        //brick =        brick_handle.get();

        //button
        Sprite_Handle  button_handle(new Sprite( textures[ID(shoot)  ].get () ));
        sprites.push_back(button_handle);
        button  =  button_handle.get ();
        button->set_position ({1000 ,600});

        //projectile
        Sprite_Handle  projectile_handle(new Sprite( textures[ID(projectile)  ].get () ));
        projectile_handle->set_position ({800, 500});
        sprites.push_back (   projectile_handle);
        projectile          =    projectile_handle.get ();
        projectile -> hide();

        //Creating aliens array
        int x, y, k=1;
        for (int i = 1; i < 4; ++i)
        {
            for (int j = 1; j < 3; ++j)
            {
                Sprite_Handle    alien_handle(new Sprite( textures[ID(alien)].get () ));
                alien_handle->set_position ({100 + i*150 , 20 + j * 150});
                sprites.push_back (   alien_handle);
                aliens[k] = alien_handle.get ();
                aliens[k]->set_speed_y(alien_speed);
               // bricks[k]->hide();
                k++;
            }
        }
    }

    //---------------------------------------------------------------------------------------------

    void Space_Scene::restart_game()
    {
        //recolocar jugador
        player->set_position ({canvas_width - player->get_width () * 3.f, canvas_height / 2.f });
        player->set_speed_y  (0.f);

        projectileShoot = false;
        follow_target = false;

        for (int i=0;i<6;i++)
        {
            alien=aliens[i+1];
            alien->show();
            alien->set_speed_y(alien_speed);
            destroyedAlienArray[i] = false;
        }

        int k=1;
        for (int i = 1; i < 4; ++i)
        {
            for (int j = 1; j < 3; ++j)
            {
                alien=aliens[k];
                alien->set_position ({100 + i*150 , 20 + j * 150});
                k++;
            }
        }

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::start_playing ()
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
            destroyedAlienArray[k]=false;
            enableAlienCollisionsArray[k]= true;
            k++;
        }

    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        update_aliens   ();
        update_user ();

        //Added
        update_projectile();

        // Se comprueban las posibles colisiones de la bola con los bordes y con los players:

        //check_collisions ();
        check_aliens_collisions();
        check_touch_input(x, y);

    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Moves the player bar
     */
    void Space_Scene::update_user ()
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
    void Space_Scene::update_projectile ()
    {
        if(!projectileShoot)
        {
            projectile->set_speed_y (player->get_speed_y());
            projectile->set_position_y (player->get_position_y());
        }

        if(projectile->intersects(*top_border))
        {
            projectile->hide();
            projectileShoot = false;
        }
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Alien movement
     * Change the alien horizontal movement when we decect a collision
     * with the borders
     */
    void Space_Scene::update_aliens ()
    {
        for (int i = 1; i < 7; i++)
        {
            alien = aliens[i];
            if (alien->intersects (*left_border) || alien->intersects (*right_border))
            {
                i = 7;
                for (int j = 1; j < 7; j++)
                {
                    alien = aliens [j];
                    alien->set_position_x(alien->get_position_x() + 25);
                    alien->set_speed_y(- alien->get_speed_y());
                }
            }

            //si cualquier alien toca al player
            float posX = alien->get_position_x();
            if(posX > player->get_position_x())
                restart_game();

            //if(alien->ge)
        }


    }

    // --------------------------------------------------------------------------------------------

    void Space_Scene::check_collisions ()
    {
            //colision con jugador/player
            if (projectile->get_right_x() > player->get_left_x())  //collision proyectil - Player
            {
                if (projectile->get_top_y() > player->get_bottom_y() &&
                    projectile->get_bottom_y() < player->get_top_y()) {
                    projectile->set_position_x(
                            player->get_left_x() - projectile->get_width() / 2.f);
                    projectile->set_speed_x(-projectile->get_speed_x());
                    //projectile->set_speed_y(-projectile->get_speed_x()*/);
                    enableAlienCollisions = true;
                    for (int i=0;i<7;++i)
                        enableAlienCollisionsArray[i]= true;
                } else    //si no choca con nosotros por debajo, hemos perdido
                    gameplay = BALL_LEAVING;
            }


    }

    // --------------------------------------------------------------------------------------------

    /*
     * Check the alien collisions with the projectile
     * We use an auxiliar var for to check if brick in our rarray of bricks
     */
    void Space_Scene::check_aliens_collisions()
    {
        if (projectileShoot) // && enableBrickCollision
        {
            int k=1;
            for (int i = 0; i < 6; i++)
            {
                 alien = aliens[k];
         //      brick->hide();
  //              projectile2->set_position ({100+k*50 , 600 });
  //              projectile2->set_speed_x  (0.f);

                //keep a copy reference to check if the speed goes right or left
                float projectile_speed_x = projectile->get_speed_x();
                float projectile_speed_y = projectile->get_speed_y();
                //comprobamos que ha llegado a su altura
                if ((projectile->get_left_x() < alien->get_right_x()&& projectile->get_right_x() > alien->get_left_x() && projectile->get_speed_x() < 0))
                {
                    if(!destroyedAlienArray[k])
                    {
                        //comprobación de colisión con stick player
                        //enableBrickCollisions = false;   //if we dont set this, the firrst if is true and keeps asking for the second condition
                        //en cuanto lo rompemos, lo ponemos a false
                        enableAlienCollisionsArray[k]= false;
                        //comprobar coordenadas
                        if (projectile->get_bottom_y() < alien->get_top_y()
                            && projectile->get_top_y() > alien->get_bottom_y()  )
                        {
                            projectile->set_speed_x(-projectile->get_speed_x());
                            alien->hide();
                            projectile->hide();
                            projectileShoot = false;
                            i = 7;  //nos salimos del bucle para ahorrar proceso

                            //si viene por debajo
                            if (projectile_speed_x > 0)
                                projectile->set_position_x(
                                        alien->get_left_x() - projectile->get_width());
                            else
                                projectile->set_position_x(
                                        alien->get_right_x() + projectile->get_width());

                            //projectile->set_speed_x(-projectile->get_speed_x());
                            //destroyedBrick = true;
                            destroyedAlienArray[k]= true;
                            for (int m = 0; m < 7; ++m) //los ponemos todos a false para que no se pueda romper otra vez ninguno
                                enableAlienCollisionsArray[m]= false;
                            bool checknoBricks = true;
                            for (int m = 1; m < 7; ++m) //true && false = false, no estan todos destruidos
                                checknoBricks= checknoBricks&&destroyedAlienArray[m];

                            //si todos han sido destruidos, el bool del array se mantiene a true
                            if (checknoBricks)
                            {
                                gameplay = BALL_LEAVING;
                                restart_game();
                            }


                        }
                    }

                }

            k++;
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Space_Scene::render_loading (Canvas & canvas)
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

    void Space_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    //--------------------------------------------------------------------------------------------

    /*
     * Instantiates a projectile
     * Shows the projectile an moves it forward to attack the aliens
     */
    void Space_Scene::Shoot()
    {
        if(!projectileShoot)
        {
            //we have already created the projectile, so we show it
            projectile->show();
            //projectile->set_speed_y (player->get_speed_y());
            projectile->set_position_y (player->get_position_y());
            projectile->set_position_x (player->get_position_x() - 40);
            //projectile->set_speed_x(-5.f);
            //proyectil sube en y
            projectile->set_speed_x(-10.f);
            projectileShoot = true;
        }
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Gets the x &y from the touch
     * For our specific object, the button, checks if the x&ytouch is between the values fo the
     * total position of the button
     */
    void Space_Scene::check_touch_input(float x, float y)
    {
        float xMin, xMax, yMin, yMax;
        xMin = button->get_position_x() - (button->get_height() / 2);
        xMax = button->get_position_x() + (button->get_height() / 2);
        yMin = button->get_position_y() - (button->get_width() / 2);
        yMax = button->get_position_y() + (button->get_width() / 2);
        if(input_button_touched
            && (x > xMin && x < xMax)
            && (y > yMin && y < yMax))
        {
            //button->set_position({xMin + 300, yMin + 300});
            input_button_touched = false;
            Shoot();
            //director.run_scene(shared_ptr< Scene >(new Intro_Scene));
        }

    }

}
