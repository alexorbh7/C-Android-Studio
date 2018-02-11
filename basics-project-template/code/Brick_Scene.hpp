
/*
 * BRICK SCENE
 *
 * alexorbh7@hotmail.com
 */

#ifndef ANDROID_STUDIO_3_BRICK_SCENE_HPP
#define ANDROID_STUDIO_3_BRICK_SCENE_HPP

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

    class Brick_Scene : public basics::Scene
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

        static constexpr float   projectile_speed = 10.f;         ///< Velocidad a la que se mueve la bola.
        static constexpr float   player_speed = 12.f;         ///< Velocidad a la que se mueven ambos jugadores.

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
        Sprite       * player;                         ///< Puntero al sprite de la lista de sprites que representa al jugador.
        Sprite       * projectile;                                ///< Puntero al sprite de la lista de sprites que representa a la bola.
        Sprite       * projectile2;                                ///< Puntero al sprite de la lista de sprites que representa a la bola.
        Sprite       * brick ;                                ///< Puntero al sprite de la lista de sprites que representa un brick.
        Sprite       * bricks [10] ;                                ///< Puntero al sprite de la lista de sprites que representa un brick.
        //Sprite_Handle  bricks_handle [];                                ///< Puntero al sprite de la lista de sprites que representa un brick.

        bool           follow_target;                       ///< true si el usuario está tocando la pantalla y su player ir hacia donde toca.
        float          user_target_x;//modified                       ///< Coordenada Y hacia donde debe ir el player del usuario cuando este toca la pantalla


        //Added to brick breaker
        bool            projectileShoot;
        bool            enableBrickCollisionsArray[6];
        bool            destroyedBrickArray[6];

        //canvas pos
        float          x, y;
        bool           input_button_touched;
        Timer          timer;

        bool           enableBrickCollisions;

        //sprites for the menus
        //sprites used for the pause menu and help menu
        Sprite       * pause_button;
        Sprite       * continue_button;
        Sprite       * exit_button;
        Sprite       * pause_menu_background;
        Sprite       * help_background;
        Sprite       * help_button;
        Sprite       * close_help_button;

        bool           game_paused=false;
        bool           enable_pause_button=false;
        bool           enable_continue_button=false;
        bool           enable_exit_button=false;
        bool           enable_help_button= false;
        bool           enable_close_help_button=false;

    public:

        /**
         * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
         * crea la escena desde cero.
         */
        Brick_Scene();

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

        //void update_bricks ();
        void update_user ();
        void update_projectile();
        void check_collisions ();
        void check_bricks_collisions ();

        void render_loading (Canvas & canvas);
        void render_playfield (Canvas & canvas);

        //added functions to brick breaker
        void Shoot();

        //check touch input
        void check_touch_exit(float x, float y);
        void check_touch_pause(float x, float y);
        void check_touch_continue(float x, float y);
        void check_touch_help(float x, float y);
        void check_touch_close_help(float x, float y);
    };

}

#endif //ANDROID_STUDIO_3_BRICK_SCENE_HPP
