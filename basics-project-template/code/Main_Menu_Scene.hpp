
/*
 * MAIN MENU SCENE
 *
 * alexorbh7@hotmail.com
 */
#ifndef ANDROID_STUDIO_3_MAIN_MENU_SCENE_HPP
#define ANDROID_STUDIO_3_MAIN_MENU_SCENE_HPP

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

    class Main_Menu_Scene : public basics::Scene
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

        State          state;                               ///< Estado de la escena.
        Gameplay_State gameplay;                            ///< Estado del juego cuando la escena está RUNNING.
        bool           paused;                              ///< true cuando la escena está en segundo plano y viceversa.

        unsigned       canvas_width;                        ///< Ancho de la resolución virtual usada para dibujar.
        unsigned       canvas_height;                       ///< Alto  de la resolución virtual usada para dibujar.

        Texture_Map    textures;                            ///< Mapa  en el que se guardan shared_ptr a las texturas cargadas.
        Sprite_List    sprites;                             ///< Lista en la que se guardan shared_ptr a los sprites creados.

        Sprite       * first_button;
        Sprite       * second_button;
        Sprite       * third_button;
        Sprite       * forth_button;
        Sprite       * scenes_buttons[5];
        Sprite       * button;

        bool           follow_target;                       ///< true si el usuario está tocando la pantalla y su player ir hacia donde toca.
        float          user_target_x;//modified                       ///< Coordenada X hacia donde debe ir el player del usuario cuando este toca la pantalla
        float          user_target_y;//modified                       ///< Coordenada X hacia donde debe ir el player del usuario cuando este toca la pantalla


        //canvas pos of touch input
        float          x, y;
        bool           input_button_touched;
        int            foodEaten;
        bool           enable_button;
        bool           player_dead;

    public:

        /**
         * Solo inicializa los atributos que deben estar inicializados la primera vez, cuando se
         * crea la escena desde cero.
         */
        Main_Menu_Scene();

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
         * Cuando se ha reiniciado el juego y el usuario toca la pantalla por primera vez se
         * pone la bola en movimiento en una dirección al azar.
         */
        void start_playing ();

        /**
         * Actualiza el estado del juego cuando el estado de la escena es RUNNING.
         */
        void run_simulation (float time);


        void render_loading (Canvas & canvas);
        void render_playfield (Canvas & canvas);

        void load_game_by_id(int sceneID);

        //check touch input
        void check_touch_input(float x, float y);

        //auxiliar not optimized code
        void load_game_1();

        //axuliar not optimized code
        void check_touch_input1(float x, float y);

    };

}

#endif //ANDROID_STUDIO_3_MAIN_MENU_SCENE_HPP
