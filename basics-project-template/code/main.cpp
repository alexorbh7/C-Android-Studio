/*
 * MAIN
 * Copyright © 2018+ Ángel Rodríguez Ballesteros
 *
 * Distributed under the Boost Software License, version  1.0
 * See documents/LICENSE.TXT or www.boost.org/LICENSE_1_0.txt
 *
 * angel.rodriguez@esne.edu
 */

#include <basics/Director>
#include <basics/enable>
#include <basics/Graphics_Resource_Cache>
#include <basics/opengles/Context>
#include <basics/Window>
#include <basics/opengles/Canvas_ES2>
#include <basics/opengles/OpenGL_ES2>

//added
#include "Brick_Scene.hpp"
#include "Intro_Scene.hpp"
#include "Snake_Scene.hpp"
#include "Game_Scene.hpp"
#include "Start_Scene.hpp"

using namespace basics;
using namespace example;
using namespace std;

int main ()
{
    // Es necesario habilitar un backend gráfico antes de nada:
    enable< basics::OpenGL_ES2 > ();

    //carga de escena del juego de snake
    director.run_scene (shared_ptr< Scene >(new Intro_Scene));

    return 0;
}

// El linker tiende a eliminar código que cree que no se usa y, cuando se termina usando, lo echa
// en falta provocando errores. Dejar estas referencias aquí por el momento para evitar esos problemas:

void keep_links ()
{
    (void)Window::can_be_instantiated;
    Window::Accessor window;
    Window::create_window (0);
    Window::get_window (0);
    Graphics_Resource_Cache cache;
    opengles::Context::create(window, &cache);
    Canvas::Factory f = opengles::Canvas_ES2::create;
    Texture_2D::register_factory (0, 0);
}
/*
void keep_links ()
{
    const bool &c = Window::can_be_instantiated;
    Window::Accessor window;
    Graphics_Resource_Cache cache;
    opengles::Context::create(window, &cache);
    Canvas::Factory f = opengles::Canvas_ES2::create;
    Window::can_be_instantiated;
    Window::get_window(0);
}
*/