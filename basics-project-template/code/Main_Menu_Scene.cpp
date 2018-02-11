//
// Created by arranque on 28/01/2018.
//


#include "Brick_Scene.hpp"
#include "Intro_Scene.hpp"
#include "Space_Scene.hpp"
#include "Snake_Scene.hpp"
#include "Game_Scene.hpp"
#include "Start_Scene.hpp"
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

    Main_Menu_Scene::Texture_Data Main_Menu_Scene::textures_data[] =
    {

        { ID(ponglogo),       "ponglogo.png" },
        { ID(ponglogo),       "ponglogo.png" },
        { ID(Snakelogo),       "Snakelogo.png" },
        { ID(bricklogo),  "bricklogo.png" },
        { ID(alienlogo),  "alien.png" },
       //  { ID(alien),    "alien.png"           },
    };

    unsigned Main_Menu_Scene::textures_count = sizeof(textures_data) / sizeof(Texture_Data);

    // ---------------------------------------------------------------------------------------------

    //Constructor
    Main_Menu_Scene::Main_Menu_Scene()
    {
        canvas_height  = 720;
        canvas_width =  1280;

        srand (unsigned(time(nullptr)));

        initialize ();

    }

    // ---------------------------------------------------------------------------------------------

    //Inicializa el juego al crearse, es como el Start de Unity
    bool Main_Menu_Scene::initialize ()
    {
        state    = LOADING;
        paused   = true;
        gameplay = UNINITIALIZED;

        return true;
    }

    // ---------------------------------------------------------------------------------------------

    void Main_Menu_Scene::suspend ()
    {
        paused = true;
    }

    // ---------------------------------------------------------------------------------------------

    void Main_Menu_Scene::resume ()
    {
        paused = false;
    }

    // ---------------------------------------------------------------------------------------------

    //Manejador de eventos del usuario
    void Main_Menu_Scene::handle (Event & event)
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

                    input_button_touched = true;

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

    void Main_Menu_Scene::update (float time)
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

    void Main_Menu_Scene::render (basics::Graphics_Context::Accessor & context)
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

    void Main_Menu_Scene::load_textures ()
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

                state = RUNNING;
            }
        }
    }

    //-----------------------------------------------------------------------------------------------

    void Main_Menu_Scene::create_sprites()
    {
        //Sprites para los bordes que matan a la serpiente
        Sprite_Handle button_1(new Sprite( textures[ID(ponglogo)].get () ));
        Sprite_Handle button_2(new Sprite( textures[ID(Snakelogo)].get () ));
        Sprite_Handle button_3(new Sprite( textures[ID(bricklogo)].get () ));
        Sprite_Handle button_4(new Sprite( textures[ID(alienlogo)].get () ));

        button_1->set_anchor   (CENTER);
        button_1->set_position ({ canvas_width / 5, canvas_height / 2.f });
        button_2->set_anchor   (CENTER);
        button_2->set_position ({ canvas_width / 5 * 2 , canvas_height / 2.f });
        button_3->set_anchor   (CENTER);
        button_3->set_position ({ canvas_width / 5 * 3 , canvas_height / 2.f });
        button_4->set_anchor   (CENTER);
        button_4->set_position ({ canvas_width / 5 * 4,  canvas_height / 2.f});

        sprites.push_back (button_1);
        sprites.push_back (button_2);
        sprites.push_back (button_3);
        sprites.push_back (button_4);

        first_button =  button_1.get ();
        second_button =  button_2.get ();
        third_button =  button_3.get ();
        forth_button =  button_4.get();


        scenes_buttons[0] = first_button;
        scenes_buttons[1]= second_button;
        scenes_buttons[2]= third_button;
        scenes_buttons[3] = forth_button;

    }

    // ---------------------------------------------------------------------------------------------

    void Main_Menu_Scene::start_playing ()
    {
        Vector2f random_direction
        (
            float(rand () % int(canvas_width ) - int(canvas_width  / 2)),
            float(rand () % int(canvas_height) - int(canvas_height / 2))
        );

        gameplay = PLAYING;
        input_button_touched = false;

    }

    // ---------------------------------------------------------------------------------------------

    /*
     * While the simulations keeps running, we keep checking the touch input
     */
    void Main_Menu_Scene::run_simulation (float time)
    {
        // Se actualiza el estado de todos los sprites:

        for (auto & sprite : sprites)
        {
            sprite->update (time);
        }

        check_touch_input(x, y);

        //not optimized
        //check_touch_input1(x, y);

    }

    // ---------------------------------------------------------------------------------------------

    void Main_Menu_Scene::render_loading (Canvas & canvas)
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

    void Main_Menu_Scene::render_playfield (Canvas & canvas)
    {
        for (auto & sprite : sprites)
        {
            sprite->render (canvas);
        }
    }

    // ---------------------------------------------------------------------------------------------

    /*
     * esta función chequea si hemos pulsado dentro de los límites de un boton en la escena
     * dependiendo de las posiciones x e y de la pulsación
     */
    void Main_Menu_Scene::check_touch_input(float x, float y)
    {

        for (int i = 0; i < 4; ++i)
        {
            button = scenes_buttons[i];
            float xMin, xMax, yMin, yMax;
            xMin = button->get_position_x() - (button->get_height() / 2);
            xMax = button->get_position_x() + (button->get_height() / 2);
            yMin = button->get_position_y() - (button->get_width() / 2);
            yMax = button->get_position_y() + (button->get_width() / 2);
            if(input_button_touched
               && (x > xMin && x < xMax)
               && (y > yMin && y < yMax))
            {
                load_game_by_id( i );
                input_button_touched = false;
            }
        }

    }

    //----------------------------------------------------------------------------------------------

    /*
     * Depending on the sceneID received, loads the appropiate scene
     */
    void Main_Menu_Scene::load_game_by_id (int sceneID)
    {
        switch (sceneID)
        {
            case 0:
                director.run_scene(shared_ptr< Scene >(new Game_Scene));
                break;
            case 1:
                director.run_scene(shared_ptr< Scene >(new Snake_Scene));
                break;
            case 2:
                director.run_scene(shared_ptr< Scene >(new Brick_Scene));
                break;
            case 3:
                director.run_scene(shared_ptr< Scene >(new Space_Scene));
                break;
            default:
                break;
        }
    }

    /*
     * Auxiliar function for testing
     */
    void Main_Menu_Scene::check_touch_input1(float x, float y)
    {
        float xMin, xMax, yMin, yMax;
        xMin = first_button->get_position_x() - (first_button->get_height() / 2);
        xMax = first_button->get_position_x() + (first_button->get_height() / 2);
        yMin = first_button->get_position_y() - (first_button->get_width() / 2);
        yMax = first_button->get_position_y() + (first_button->get_width() / 2);
        if(input_button_touched
           && (x > xMin && x < xMax)
           && (y > yMin && y < yMax))
        {
            load_game_1( );
            input_button_touched = false;
        }

    }

    /*
     * Auxiliar function for  testing
     */
    void Main_Menu_Scene::load_game_1 ()
    {
        director.run_scene(shared_ptr< Scene >(new Game_Scene));

    }

}
