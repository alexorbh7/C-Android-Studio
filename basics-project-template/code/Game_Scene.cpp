/*
 * GAME SCENE
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

#include "Game_Scene.hpp"

#include <cstdlib>
#include <basics/Canvas>
#include <basics/Director>
#include <basics/Log>
#include <basics/Scaling>
#include <basics/Rotation>
#include <basics/Translation>

using namespace basics;
using namespace std;

namespace example
{

    Game_Scene::Texture_Data Game_Scene::textures_data[] =
    {
        { ID(loading),    "game-scene/loading.png"        },
        { ID(hbar),       "game-scene/horizontal-bar.png" },
        { ID(vbar),       "game-scene/vertical-bar.png"   },
        { ID(player-bar), "game-scene/players-bar.png"    },
        { ID(ball),       "game-scene/ball.png"           },
        { ID(wall),       "game-scene/players-bar.png"           },
    };

    unsigned Game_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------

    constexpr float Game_Scene::  ball_speed;
    constexpr float Game_Scene::player_speed;

    // ---------------------------------------------------------------------------------------------

    //Constructor
    Game_Scene::Game_Scene()
    {
        canvas_width  = 1280;
        canvas_height =  720;

        srand (unsigned(time(nullptr)));

        initialize ();

        //outsidePositionX = 100.f;
        //outsidePositionY = 100.f;
    }

    // ---------------------------------------------------------------------------------------------

    bool Game_Scene::initialize ()
    {
        state    = LOADING;
        paused   = true;
        gameplay = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::suspend ()
    {
        paused = true;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::resume ()
    {
        paused = false;
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::handle (Event & event)
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
                    break;
                }
                case ID(touch-moved):
                {
                    user_target_y = *event[ID(y)].as< var::Float > ();
                    follow_target = true;
                    break;
                }

                case ID(touch-ended):
                {
                    follow_target = false;
                    break;
                }

            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::update (float time)
    {
        if (!paused)
            switch (state)
            {
                case LOADING: load_textures  ();     break;
                case RUNNING: run_simulation (time); break;
                case ERROR:   break;
            }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render (basics::Graphics_Context::Accessor & context)
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

    void Game_Scene::load_textures ()
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

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::create_sprites ()
    {
        // Se crean y configuran los sprites del fondo:

        Sprite_Handle    top_bar(new Sprite( textures[ID(hbar)].get () ));
        Sprite_Handle middle_bar(new Sprite( textures[ID(vbar)].get () ));
        Sprite_Handle bottom_bar(new Sprite( textures[ID(hbar)].get () ));

           top_bar->set_anchor   (TOP | LEFT);
           top_bar->set_position ({ 0, canvas_height });
        middle_bar->set_anchor   (CENTER);
        middle_bar->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
        bottom_bar->set_anchor   (BOTTOM | LEFT);
        bottom_bar->set_position ({ 0, 0 });

        sprites.push_back (   top_bar);
        sprites.push_back (middle_bar);
        sprites.push_back (bottom_bar);

        // Se crean los players y la bola:

        Sprite_Handle  left_player_handle(new Sprite( textures[ID(player-bar)].get () ));
        Sprite_Handle right_player_handle(new Sprite( textures[ID(player-bar)].get () ));
        Sprite_Handle         ball_handle(new Sprite( textures[ID(ball)      ].get () ));

        sprites.push_back ( left_player_handle);
        sprites.push_back (right_player_handle);
        sprites.push_back (        ball_handle);

        // Se guardan punteros a los sprites que se van a usar frecuentemente:

        top_border    =             top_bar.get ();
        bottom_border =          bottom_bar.get ();
        left_player   =  left_player_handle.get ();
        right_player  = right_player_handle.get ();
        ball          =         ball_handle.get ();

    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::restart_game()
    {
         left_player->set_position ({ left_player->get_width () * 3.f, canvas_height / 2.f });
         left_player->set_speed_y  (0.f);
        right_player->set_position ({ canvas_width  - right_player->get_width () * 3.f, canvas_height / 2.f });
        right_player->set_speed_y  (0.f);
                ball->set_position ({ canvas_width / 2.f, canvas_height / 2.f });
                ball->set_speed    ({ 0.f, 0.f });

        follow_target = false;

        gameplay = WAITING_TO_START;

        //added to check collisions with instantiated walls
       // InstantiateWall();
        //enableWallCollisions = false;

    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::start_playing ()
    {
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        ball->set_speed (random_direction.normalized () * ball_speed);

        gameplay = PLAYING;

        //added to check collisions with instantiated walls
        enableWallCollisions = false;

        instantiatedWall = true;
        //added new wall
        InstantiateWall();
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        update_ai   ();
        update_user ();

        // Se comprueban las posibles colisiones de la bola con los bordes y con los players:

        check_collisions ();
        check_projectile_collisions ();
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Moves the Ai dependeng on the player bar position
     */
    void Game_Scene::update_ai ()
    {
        if (left_player->intersects (*top_border))
        {
            left_player->set_position_y (top_border->get_bottom_y () - left_player->get_height () / 2.f);
            left_player->set_speed_y (0.f);
        }
        else
        if (left_player->intersects (*bottom_border))
        {
            left_player->set_position_y (bottom_border->get_top_y () + left_player->get_height () / 2.f);
            left_player->set_speed_y (0.f);
        }
        else
        {
            float delta_y = ball->get_position_y () - left_player->get_position_y ();

            if (ball->get_speed_y () < 0.f)
            {
                if (delta_y < 0.f)
                {
                    left_player->set_speed_y (-player_speed * (ball->get_speed_x () < 0.f ? 1.f : .5f));
                }
                else
                    left_player->set_speed_y (0.f);
            }
            else
            if (ball->get_speed_y () > 0.f)
            {
                if (delta_y > 0.f)
                {
                    left_player->set_speed_y (+player_speed * (ball->get_speed_x () < 0.f ? 1.f : .5f));
                }
                else
                    left_player->set_speed_y (0.f);
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Updates/moves the position of the player
     */
    void Game_Scene::update_user ()
    {
        if (right_player->intersects (*top_border))
        {
            right_player->set_position_y (top_border->get_bottom_y () - right_player->get_height () / 2.f);
            right_player->set_speed_y (0);
        }
        else
        if (right_player->intersects (*bottom_border))
        {
            right_player->set_position_y (bottom_border->get_top_y () + right_player->get_height () / 2.f);
            right_player->set_speed_y (0);
        }
        else
        if (follow_target)
        {
            float delta_y = user_target_y - right_player->get_position_y ();

            if (delta_y < 0.f) right_player->set_speed_y (-player_speed); else
            if (delta_y > 0.f) right_player->set_speed_y (+player_speed);
        }
        else
            right_player->set_speed_y (0);
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::check_collisions ()
    {
        // Se comprueba si la bola choca con el borde superior o con el inferior, en cuyo caso se
        // ajusta su posición para que no lo atraviese y se invierte su velocidad en el eje Y:

        if (ball->intersects (*top_border))
        {
            ball->set_position_y (top_border->get_bottom_y () - ball->get_height() / 2.f);
            ball->set_speed_y    (-ball->get_speed_y ());
        }

        if (ball->intersects (*bottom_border))
        {
            ball->set_position_y (bottom_border->get_top_y () + ball->get_height() / 2.f);
            ball->set_speed_y    (-ball->get_speed_y ());
        }

        // Solo si la bola no ha superado alguno de los players, se comprueba si choca con alguno
        // de ellos, en cuyo caso se ajusta su posición para que no los atraviese y se invierte su
        // velocidad en el eje X:

        if (gameplay != BALL_LEAVING)
        {
            //AI, enemigo
            if (ball->get_left_x () < left_player->get_right_x ())  //si hay colision bola-AI
            {
                if (ball->get_bottom_y () < left_player->get_top_y () && ball->get_top_y () > left_player->get_bottom_y ())
                {
                    ball->set_position_x (left_player->get_right_x () + ball->get_width() / 2.f);
                    ball->set_speed_x    (-ball->get_speed_x ());
                    enableWallCollisions = true;
                }
                else
                    gameplay = BALL_LEAVING;
            }

            if (ball->get_right_x () > right_player->get_left_x ())//jugador
            {
                //comprobación de colisión con stick
                if (ball->get_bottom_y () < right_player->get_top_y () && ball->get_top_y () > right_player->get_bottom_y ())
                {
                    ball->set_position_x (right_player->get_left_x () - ball->get_width() / 2.f);
                    ball->set_speed_x    (-ball->get_speed_x ());
                    enableWallCollisions = true;
                    //función relocate AI
                    MoveAI();


                }
                else
                    gameplay = BALL_LEAVING;
            }

            if(enableWallCollisions && !instantiatedWall) //Wall collisions
            {
                //keep a copy reference to check if the speed goes right or left
                float ball_speed_x = ball->get_speed_x();
                if ((ball->get_right_x () > wall->get_left_x () && ball_speed_x > 0)
                    ||  (ball->get_left_x () < wall->get_right_x () && ball_speed_x < 0) )
                {
                    //comprobación de colisión con stick
                    enableWallCollisions = false;   //if we dont set this, the firrst if is true and keeps asking for the second condition
                    if (ball->get_bottom_y () < wall->get_top_y ()
                        && ball->get_top_y () > wall->get_bottom_y ())
                    {
                        if(ball_speed_x > 0)
                            ball->set_position_x (wall->get_left_x () - ball->get_width() );
                        else
                            ball->set_position_x (wall->get_right_x () + ball->get_width() );
                        ball->set_speed_x    (-ball->get_speed_x ());
                        enableWallCollisions = false;
                    }
                }
            }
        }
        else
        if (ball->get_right_x () < 0.f || ball->get_left_x () > float(canvas_width))
        {
            restart_game ();
        }
    }

    //--------------------------------------------------------------------------------------------------------------

    ///Check the projectile collisions in the scene
    void Game_Scene::check_projectile_collisions ()
    {
        if(enableProjectileCollisions)
        {
            if (projectile->get_left_x () < left_player->get_right_x ())  //si hay colision bola-AI
            {
                if (projectile->get_bottom_y() < left_player->get_top_y() &&
                    projectile->get_top_y() > left_player->get_bottom_y())
                {
                    //Destroy projectile
                    projectile->hide();
                    enableProjectileCollisions = false;
                    //projectile->set_speed_x(15.f);
                }

            }

            if (projectile->get_left_x () < wall->get_right_x () && !instantiatedWall)  //si hay colision con muro
            {
                if (projectile->get_bottom_y() < wall->get_top_y() &&
                    projectile->get_top_y() > wall->get_bottom_y())
                {
                    //Destroy projectile
                    wall->hide();
                    //wall->set_position_x(outsidePositionX);
                    //wall->set_position_y(outsidePositionY);
                    projectile -> hide();
                    wall-> hide();
                    enableProjectileCollisions = false;
                    enableWallCollisions = false;
                    instantiatedWall =true;
                    //enableWallCollisions = false;
                    //projectile->set_speed_x(15.f);
                }
            }

            if (projectile->get_right_x() < 0.f || projectile->get_left_x() > float(canvas_width))
            {
                enableProjectileCollisions = false;
                projectile -> hide();
            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Game_Scene::render_loading (Canvas & canvas)
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

    void Game_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Difficulty added: the oponent advance forward
     */
    void Game_Scene::MoveAI()
    {
        float currentPosX = left_player->get_position_x();
        if(currentPosX < 500)
        left_player->set_position ({ left_player->get_width () * 3.f + currentPosX, left_player->get_position_y()});
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * Creates a wall in the middle of the scene
     */
    void Game_Scene::InstantiateWall()
    {
        if (instantiatedWall)
        {
            Sprite_Handle wall_handle(new Sprite(textures[ID(wall)].get()));
            wall_handle->set_anchor(TOP | LEFT);
            wall_handle->set_position({canvas_width / 2, canvas_height / 2 - 50.0f});
            sprites.push_back(wall_handle);
            wall = wall_handle.get();
            instantiatedWall = false; // Hay muro
        }
    }

    //--------------------------------------------------------------------------------------------
    /*
     * Instantiates a projectile to destroy the middle wall
     */
    void Game_Scene::Shoot()
    {
        if(!enableProjectileCollisions)
        {
            Sprite_Handle    projectile_handle(new Sprite( textures[ID(ball)].get () ));
            //projectile_handle->set_anchor   (TOP | LEFT);
            projectile_handle->set_position ({(right_player->get_left_x () - ball->get_width() / 2.f), right_player->get_position_y ()});
            sprites.push_back (   projectile_handle);
            projectile          =    projectile_handle.get ();

            projectile->set_speed_x    (-15.f);
            enableProjectileCollisions = true;
        }

    }

}
