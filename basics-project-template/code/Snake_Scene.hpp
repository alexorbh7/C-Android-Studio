//
// Created by arranque on 28/01/2018.
//

#ifndef ANDROID_STUDIO_3_SNAKE_SCENE_HPP
#define ANDROID_STUDIO_3_SNAKE_SCENE_HPP

#include <map>
#include <list>
#include <memory>
#include <basics/Canvas>
#include <basics/Id>
#include <basics/Scene>
#include <basics/Texture_2D>
#include <basics/Timer>

#include "Sprite.hpp"

namespace example
{

    using basics::Id;
    using basics::Canvas;
    using basics::Texture_2D;
    using basics::Timer;

    class Snake_Scene : public basics::Scene
    {

        // Estos typedefs pueden ayudar a hacer el código más compacto y claro:

        typedef std::shared_ptr < Sprite     >     Sprite_Handle;
        typedef std::list< Sprite_Handle     >     Sprite_List;
        typedef std::shared_ptr< Texture_2D  >     Texture_Handle;
        typedef std::map< Id, Texture_Handle >     Texture_Map;
        typedef basics::Graphics_Context::Accessor Context;

        /**
         * Representa el estado de la escena en su conjunto.
         */
        enum State
        {
            LOADING,
            RUNNING,
            ERROR
        };

        /**
         * Representa el estado del juego cuando el estado de la escena es RUNNING.
         */
        enum Gameplay_State
        {
            UNINITIALIZED,
            WAITING_TO_START,
            PLAYING,
            //CHANGE STATES OF GAME
            BALL_LEAVING,
        };

    private:

        /**
         * Array de estructuras con la información de las texturas (Id y ruta) que hay que cargar.
         */
        static struct   Texture_Data { Id id; const char * path; } textures_data[];

        /**
         * Número de items que hay en el array textures_data.
         */
        static unsigned textures_count;

    private:

        static constexpr float   snake_speed = 4.f;         ///< Velocidad a la que se mueve el jugador

    private:

        State          state;                               ///< Estado de la escena.
        Gameplay_State gameplay;                            ///< Estado del juego cuando la escena está RUNNING.
        bool           paused;                              ///< true cuando la escena está en segundo plano y viceversa.

        unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
        unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.

        Texture_Map    textures;                            ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
        Sprite_List    sprites;                             ///< Lista en la que se guardan shared_ptr a los sprites creados.

        Sprite       * top_border;                          ///< Puntero al sprite de la lista de sprites que representa el borde superior.
        Sprite       * left_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde izq.
        Sprite       * right_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde der.
        Sprite       * bottom_border;                       ///< Puntero al sprite de la lista de sprites que representa el borde der.
        Sprite       * snake ;                                ///< Puntero al sprite de la lista de sprites que representa un brick.
        Sprite       * food ;                                ///< Puntero al sprite de la lista de sprites que representa un brick.
        Sprite       * button;

        //canvas pos of touch input
        float          x, y;
        bool           input_button_touched;
        int            foodEaten;
        bool           enable_button;
        bool           player_dead;

        //variables for food and snake grow
        Sprite        * snake_test_grow;
        Sprite        * snake_grow[20];
        int             total_grow;
        int             pixeles;
        float           target_pos_x [10000], target_pos_y [1000];

    public:

        /**
         * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
         * crea la escena desde cero.
         */
        Snake_Scene();

        /**
         * Este método lo llama Director para conocer la resolución virtual con la que está
         * trabajando la escena.
         * @return Tamaño en coordenadas virtuales que está usando la escena.
         */
        basics::Size2u get_view_size () override
        {
            return { canvas_width, canvas_height };
        }

        /**
         * Aquí se deberían inicializar los atributos que deben restablecerse cada vez que se
         * inicia la escena (si una misma escena se inicia más de una vez).
         * @return
         */
        bool initialize () override;

        /**
         * Este método lo invoca Director automáticamente cuando el juego pasa a segundo plano.
         */
        void suspend () override;

        /**
         * Este método lo invoca Director automáticamente cuando el juego pasa a primer plano.
         */
        void resume () override;

        /**
         * Este método se invoca automáticamente una vez por fotograma cuando se acumulan
         * eventos dirigidos a la escena.
         */
        void handle (basics::Event & event) override;

        /**
         * Este método se invoca automáticamente una vez por fotograma para que la escena
         * actualize su estado.
         */
        void update (float time) override;

        /**
         * Este método se invoca automáticamente una vez por fotograma para que la escena
         * dibuje su contenido.
         */
        void render (Context & context) override;

    private:

        /**
         * En este método se cargan las texturas (una cada fotograma para facilitar que la
         * propia carga se pueda pausar cuando la aplicación pasa a segundo plano).
         */
        void load_textures ();

        /**
         * En este método se crean los sprites cuando termina la carga de texturas.
         */
        void create_sprites ();

        /**
         * Se llama cada vez que se debe reiniciar el juego. En concreto la primera vez y cada
         * vez que un jugador pierde.
         */
        void restart_game ();

        /**
         * Cuando se ha reiniciado el juego y el usuario toca la pantalla por primera vez se
         * pone la bola en movimiento en una dirección al azar.
         */
        void start_playing ();

        /**
         * Actualiza el estado del juego cuando el estado de la escena es RUNNING.
         */
        void run_simulation (float time);

        /*
        void update_aliens ();
        void update_user ();
        void update_projectile();
        void check_collisions ();
        void check_aliens_collisions ();
         */

        void render_loading (Canvas & canvas);
        void render_playfield (Canvas & canvas);

        void check_snake_collisions();
        void check_touch_direction(float x, float y);
        void increment_snake(int food_count);
        void snake_movement();


        //check touch input
        void check_touch_input(float x, float y);


    };

}

#endif //ANDROID_STUDIO_3_SNAKE_SCENE_HPP
