
/*
 * SNAKE SCENE
 *
 * alexorbh7@hotmail.com
 */
#include "Brick_Scene.hpp"
#include "Intro_Scene.hpp"
#include "Space_Scene.hpp"
#include "Snake_Scene.hpp"
#include "Start_Scene.hpp"

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

    Snake_Scene::Texture_Data Snake_Scene::textures_data[] =
    {
        { ID(loading),    "game-scene/loading.png"        },
        { ID(hbar),       "game-scene/horizontal-bar.png" },
        { ID(vbar),       "game-scene/vertical-bar.png"   },
        { ID(verticalbar),       "game-scene/new-vertical-bar.png"   },
        { ID(button),       "alien.png"           },
        { ID(snake),        "alien.png"           },
        { ID(food),        "food2.png"           },
        { ID(logo),         "death.png"           },
    };

    unsigned Snake_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------

    //cambiar en el hpp
    constexpr float Snake_Scene::   snake_speed;

    // ---------------------------------------------------------------------------------------------

    //Constructor
    Snake_Scene::Snake_Scene()
    {
        canvas_height  = 720;
        canvas_width =  1280;

        srand (unsigned(time(nullptr)));

        initialize ();

    }

    // ---------------------------------------------------------------------------------------------

    //Inicializa el juego al crearse, es como el Start de Unity
    bool Snake_Scene::initialize ()
    {
        state    = LOADING;
        paused   = true;
        gameplay = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::suspend ()
    {
        paused = true;
    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::resume ()
    {
        paused = false;
    }

    // ---------------------------------------------------------------------------------------------

    //Manejador de eventos del usuario
    void Snake_Scene::handle (Event & event)
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
                    input_button_touched = true;
                    x = *event[ID(x)].as< var::Float > ();
                    y = *event[ID(y)].as< var::Float > ();


                    //si el jugador está muerto y toca la pantalla, restart
                    if(player_dead)
                        restart_game();
                    break;
                }
                case ID(touch-moved):
                {
                    //user_target_x = *event[ID(y)].as< var::Float > ();
                    input_button_touched = false;
                    //follow_target = true;
                    break;
                }

                case ID(touch-ended):
                {
                    input_button_touched = false;
                    //follow_target = false;
                    break;
                }

            }
        }
    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::update (float time)
    {
        if (!paused)
            switch (state)
            {
                case LOADING: load_textures  ();     break;
                case RUNNING:
                    //si el player esta muerto dejamos de correr la simulación
                    //esperando a que toque la pantalla
                    if(!player_dead)
                        run_simulation (time);
                    break;
                case ERROR:   break;
            }
    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::render (basics::Graphics_Context::Accessor & context)
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

    //carga de las texturas del juego
    void Snake_Scene::load_textures ()
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

    void Snake_Scene::create_sprites()
    {
        //Sprites para los bordes que matan a la serpiente
        Sprite_Handle    top_bar(new Sprite( textures[ID(hbar)].get () ));
        Sprite_Handle middle_bar(new Sprite( textures[ID(verticalbar)].get () ));
        Sprite_Handle right_bar (new Sprite( textures[ID(verticalbar)].get () ));
        Sprite_Handle bottom_bar(new Sprite( textures[ID(hbar)].get () ));

        top_bar->set_anchor   (TOP | LEFT);
        top_bar->set_position ({ 0, canvas_height });
        middle_bar->set_anchor   (CENTER);
        middle_bar->set_position ({ 0.f , canvas_height / 2.f });
        right_bar->set_anchor   (CENTER);
        right_bar->set_position ({ canvas_width - 10.f , canvas_height / 2.f });
        bottom_bar->set_anchor   (BOTTOM | LEFT);
        bottom_bar->set_position ({ 0, 0 });

        sprites.push_back (   top_bar);
        sprites.push_back (middle_bar);
        sprites.push_back (right_bar);
        sprites.push_back (bottom_bar);

        //Sprites de juego: snake
        Sprite_Handle  snake_handle(new Sprite( textures[ID(snake)].get () ));
        sprites.push_back(snake_handle);

        //Punteros para los sprites que usemos con frecuencia
        top_border  =  middle_bar.get ();
        left_border =  bottom_bar.get ();
        right_border=  top_bar.get ();
        bottom_border = right_bar.get();
        snake  =      snake_handle.get ();

        //button: mensaje para el jugador cuando muere
        Sprite_Handle  button_handle(new Sprite( textures[ID(logo)  ].get () ));
        sprites.push_back(button_handle);
        button  =  button_handle.get ();

        //lo ponemos en el centro de la pantalla
        middle_bar->set_anchor   (CENTER);
        button->set_position ({canvas_width / 2.f , canvas_height / 2.f});
        button->hide();
        enable_button = false;

        //food: comida para la serpiente: lo alineamos al principio
        //para una primera comprobación del juego
        Sprite_Handle  food_handle(new Sprite( textures[ID(food)  ].get () ));
        food_handle->set_position ({800, 500});
        sprites.push_back (   food_handle);
        food          =    food_handle.get ();

    }

    void Snake_Scene::restart_game()
    {

        //recoloca la comida y la serpiente al inicio de la partida
        //snake->set_position(random_position);
        snake->set_position ({500 , 500});

        snake->set_speed_x ({snake_speed});
        snake->set_speed_y ({0.f});

        food->set_position ({300 , 300});

        for (int i = 0; i < total_grow; ++i)
        {
            snake_grow[i] -> hide();
        }

        foodEaten = 0;

        button->hide();
        enable_button = false;
        player_dead = false;

        total_grow = 0;
        pixeles = 0;

        gameplay = WAITING_TO_START;
    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::start_playing ()
    {
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        gameplay = PLAYING;
        input_button_touched = false;
        enable_button = false;
        player_dead = false;

        total_grow = 0;
        pixeles = 0;

    }

    // ---------------------------------------------------------------------------------------------

    //nuestro update del juego: chequea continuamente lo que le pasemos
    void Snake_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        check_touch_direction(x , y);
        check_snake_collisions();
        snake_movement();
    }

    // ---------------------------------------------------------------------------------------------

    //comprobacion de colisiones de la serpiente
    void Snake_Scene::check_snake_collisions()
    {
        if (snake->intersects (*food))
        {
            //cambiamos la comida a una posicion random pero dentro de nuestros límites
            food->set_position({rand() % 1280 , rand()% 720});
            //snake->set_speed_x(-snake->get_speed_x());
            //snake->set_speed_y(-snake->get_speed_y());
            foodEaten ++;
            increment_snake(foodEaten);
        }

        if (snake->intersects (*top_border))
        {
            enable_button = true;
            button->show();
            player_dead = true;
            //director.run_scene (shared_ptr< Scene >(new Start_Scene));
        }

        if (snake->intersects (*left_border))
        {
            enable_button = true;
            button->show();
            player_dead = true;
            //director.run_scene (shared_ptr< Scene >(new Start_Scene));
        }

        if (snake->intersects (*right_border))
        {
            enable_button = true;
            button->show();
            player_dead = true;
            //director.run_scene (shared_ptr< Scene >(new Start_Scene));
        }

        if (snake->intersects (*bottom_border))
        {
            enable_button = true;
            button->show();
            player_dead = true;
            //director.run_scene (shared_ptr< Scene >(new Start_Scene));
        }

    }

    // ---------------------------------------------------------------------------------------------

    /*
     * cambia la direccion de la serpiente dependiendo de
     * 1- la velocidad: el eje en el que se esta moviendo
     * 2- la posición en la pantalla (si hemos tocado a un lado u otro / sobre o bajo el alien)
     */
    void Snake_Scene::check_touch_direction(float x, float y)
    {

        if(input_button_touched)
        {

            if(snake->get_speed_x() > 0 && snake->get_speed_y() == 0) //se mueve en el eje x + der
            {
                //invertimos a eje y
                if( y > snake->get_position_y()) //si se ha pulsado por encima
                {
                    snake->set_speed_y(snake_speed);
                    snake->set_speed_x(0.f);
                }
                else //y < snake y
                if( y < snake->get_position_y())// si se ha pulsado por debajo
                {
                    snake->set_speed_y(-snake_speed);
                    snake->set_speed_x(0.f);
                }
            }
            else
            if(snake->get_speed_x() < 0 && snake->get_speed_y() == 0) //se mueve en el eje x - iqz
            {
                //invertimos a eje y
                if( y > snake->get_position_y()) //si se ha pulsado por encima
                {
                    snake->set_speed_y(snake_speed);
                    snake->set_speed_x(0.f);
                }
                else //y < snake y
                if(y < snake->get_position_y())// si se ha pulsado por debajo
                {
                    snake->set_speed_y(-snake_speed);
                    snake->set_speed_x(0.f);
                }
            }
            else
            if(snake->get_speed_y() > 0 && snake->get_speed_x() == 0) //se mueve en el eje y + up
            {
                //invertimos a eje x
                if( x > snake->get_position_x()) //si se ha pulsado por derecha
                {
                    snake->set_speed_x(snake_speed);
                    snake->set_speed_y(0.f);
                }
                else //x < snake x
                if( x < snake->get_position_x())// si se ha pulsado por izq
                {
                    snake->set_speed_x(-snake_speed);
                    snake->set_speed_y(0.f);
                }
            }
            else
            if(snake->get_speed_y() < 0 && snake->get_speed_x() == 0) //se mueve en el eje y - down
            {
                //invertimos a eje x
                if( x > snake->get_position_x()) //si se ha pulsado por der
                {
                    snake->set_speed_x(snake_speed);
                    snake->set_speed_y(0.f);
                }
                else
                if( x < snake->get_position_x())// si se ha pulsado por izq
                {
                    snake->set_speed_x(-snake_speed);
                    snake->set_speed_y(0.f);
                }
            }
        }



        /* borrador de seguridad del planteamiento
        if(snake->get_speed_x() > 0)//se mueve en el eje x +
        {
            //invertimos a eje y
            if( y > snake->get_position_y()) //si se ha pulsado por encima
            {
                snake->set_speed_y(snake_speed);
                snake->set_speed_x(0.f);
            } else //y < snake y
            if(y > snake->get_position_y())// si se ha pulsado por debajo
            {
                snake->set_speed_y(-snake_speed);
                snake->set_speed_x(0.f);
            }
        }
         */
        input_button_touched = false;
    }

    // ---------------------------------------------------------------------------------------------

    //comprobacion de colisiones de la serpiente
    void Snake_Scene::increment_snake(int food_count)
    {
        Sprite_Handle  new_food(new Sprite( textures[ID(food)].get () ));
        new_food->set_anchor (CENTER);
       // new_food->set_position ({target_pos_x[pixeles-20], target_pos_y[pixeles-20]});
        sprites.push_back (new_food);
        snake_test_grow = new_food.get ();

        snake_grow [total_grow] = snake_test_grow;
        total_grow++;

    }

    // ---------------------------------------------------------------------------------------------

    //comprobacion de colisiones de la serpiente
    void Snake_Scene::snake_movement()
    {
        pixeles++;
        if (pixeles>=0)
        {
            if(total_grow > 0)
            {
                snake_grow[0]->set_position_x({target_pos_x[pixeles-20]});
                snake_grow[0]->set_position_y({target_pos_y[pixeles-20]});
            }

            //move all the snake

            for (int i = 0; i < total_grow; i++)
            {
                target_pos_x[i] = snake_grow[i-1]->get_position_x();
                target_pos_y[i] = snake_grow[i-1]->get_position_y();
                //target_pos_y[i] = target_pos_y[i-1];
                snake_grow[i]->set_position_x({target_pos_x[pixeles-(i+1)*20]});
                snake_grow[i]->set_position_y({target_pos_y[pixeles-(i+1)*20]});
            }


        }
        //pixeles=0;
        target_pos_x[pixeles] = snake->get_position_x();
        target_pos_y[pixeles] = snake->get_position_y();


    }

    // ---------------------------------------------------------------------------------------------

    void Snake_Scene::render_loading (Canvas & canvas)
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

    void Snake_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------

    //esta función chequea si hemos pulsado dentro de los límites de un boton en la escena
    //dependiendo de las posiciones x e y de la pulsación
    void Snake_Scene::check_touch_input(float x, float y)
    {
        if(enable_button)
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
                player_dead = false;
                restart_game();
                input_button_touched = false;
                //director.run_scene(shared_ptr< Scene >(new Start_Scene));
            }
        }
        enable_button = false;
    }
}
