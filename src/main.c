#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include "framework.h"
#include "chip8.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "../vendor/nuklear.h"
#include "../vendor/nuklear_sdl_gl3.h"

#define WINDOW_WIDTH (SCREEN_WIDTH * SCREEN_SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * SCREEN_SCALE)

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

struct nk_context *nk_ctx;
struct nk_colorf nuk_bg;
SDL_Window *win = NULL;
DoxieRenderer renderer;
SDL_GLContext gl_ctx;
bool render_quad = true;

bool SetupSDL( const char* window_name );
//bool SetupGL( void );
void HandleKeys( u8 key, int x, int y );
void Update( void );
void Render( void );
void Terminate( void );

/**
 * Pass success pointer NULL if error is just a warning.
 */
void PrintSDLError( const char *msg, bool *success ) {
		if ( success == NULL ) {
				fprintf( stderr, "[sdl] Warning: %s! SDL_Error: %s\n", msg, SDL_GetError() );
		} else {
				fprintf( stderr, "[sdl] %s! SDL_Error: %s\n", msg, SDL_GetError() );
				*success = false;
		}
}

bool SetupSDL( const char* window_name ) {
		bool success = true;
		
		SDL_SetHint( SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0" );
		if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0 ) {
				PrintSDLError( "SDL could not initialize", &success );
		} else {
				SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
				SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

				win = SDL_CreateWindow( window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI );
				if ( win == NULL ) {
						PrintSDLError( "Window could not be created", &success );
				} else {
						gl_ctx = SDL_GL_CreateContext( win );
						if ( gl_ctx == NULL ) {
								PrintSDLError( "OpenGL context could not be created", &success );
						} else {
								glViewport( 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT );
								glewExperimental = GL_TRUE;
								GLenum glewError = glewInit();
								if ( glewError != GLEW_OK ) {
										fprintf( stderr, "[glew] Error initializing GLEW! %s\n", glewGetErrorString(glewError) );
										success = false;
								}

								// VSync
								if ( SDL_GL_SetSwapInterval(1) < 0 ) {
										PrintSDLError( "Unable to set VSync", NULL );
								}

								/*if ( !SetupGL() ) {
										fprintf( stderr, "[ogl] Unable to initialize OpenGL!\n" );
										success = false;
								}*/

								if ( !Doxie_RenderInit(&renderer, WINDOW_WIDTH, WINDOW_HEIGHT ) ) {
										fprintf( stderr, "[ogl] Unable to initialize OpenGL!\n" );
										success = false;
								}

								nk_ctx = nk_sdl_init( win );

								struct nk_font_atlas *atlas;
								nk_sdl_font_stash_begin(&atlas);
								nk_sdl_font_stash_end();
						}
				}
		}
		return success;
}

/*bool SetupGL( void ) {
		bool success = true;


else {

								gl_vertex_pos_2d_loc = glGetAttribLocation( gl_program_id, "LVertexPos2D" );
								if ( gl_vertex_pos_2d_loc == -1 ) {
										fprintf( stderr, "LVertexPos2D is not a valid glsl program variable!\n" );
										success = false;
								} else {

										GLfloat vdata[] = {
												-0.5f, -0.5f,
												0.5f, -0.5f,
												0.5f, 0.5f,
												-0.5f, 0.5f
										};

										GLuint idata[] = { 0, 1, 2, 3 };

										glGenBuffers( 1, &VBO );
										glGenBuffers( 1, &IBO );
										glGenVertexArrays( 1, &VAO );

										glBindBuffer( GL_ARRAY_BUFFER, VBO );
										glBufferData( GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_STATIC_DRAW );

										glVertexAttribPointer( gl_vertex_pos_2d_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0 );
										glEnableVertexAttribArray( gl_vertex_pos_2d_loc );

										glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
										glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), idata, GL_STATIC_DRAW );

								}
						}

				}
		}

		return success;
}*/

void HandleKeys( u8 key, int x, int y ) {
		if ( key == 'q' ) {
				render_quad = !render_quad;
		}
} 

void Update() {

}

u16 c8_pixel_size = 1 * SCREEN_SCALE;

void C8RenderDisplay( void ) {
		
		for ( u16 i = 0; i < SCREEN_SIZE; i++ ) {
				if ( gfx[i] == 1 ) {
						u16 x = (i % SCREEN_WIDTH) * c8_pixel_size;
						u16 y = (i / SCREEN_WIDTH) * c8_pixel_size;
						Doxie_RenderPushQuadT( &renderer,
						(rect){
								x,
								y,
								c8_pixel_size,
								c8_pixel_size
						},
						DoxWhite
						);
				}
		}
}

void Render( void ) {
		Doxie_RenderBeginFrame( &renderer );
		/*Doxie_RenderPushQuadT( &renderer,
						(rect){
								( WINDOW_WIDTH - 100.f) / 2.f,
								( WINDOW_HEIGHT - 100.f) / 2.f,
								100.f,
								100.f
						},
						$(vec4, 1, 0, 0, 1)
						);
		Doxie_RenderPushTriangle( &renderer, 
								$(vec2, 100.f, 100.f),
								$(vec2, 300.f, 100.f),
								$(vec2, 200.f, 300.f),
								$(vec4, 1, 0, 0, 1), $(vec4, 1, 0, 0, 1), $(vec4, 1, 0, 0, 1) );*/

		C8RenderDisplay();

		/*if ( render_quad ) {
				glUseProgram( gl_program_id );
				glBindVertexArray( VAO );

				glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL );

				//glDisableVertexAttribArray( gl_vertex_pos_2d_loc );
				//glUseProgram( (void*)0 );
		}*/
		Doxie_RenderEndFrame( &renderer );
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
		//SDL_GL_SwapWindow( win );
}

void PrintInfo( u32 interval, void *param ) {
		Log( LTRACE, "Current number of triangles: %d", renderer.triangle_count );
}

// Not the safest 
void FormatString( char *buf, const char *fmt, ... ) {
		va_list vargs;
		va_start( vargs, fmt );
		vsprintf( buf, fmt, vargs );
		va_end( vargs );
}

int main( int argc, char **argv ) {
		LogInit( "c8" );

		if ( !SetupSDL( "CHIP-8" ) ) {
				fprintf( stderr, "[c8] Failed to initialize SDL!\n" );
				return EXIT_FAILURE;
		}
		if ( C8Setup() != 0 ) {
				fprintf( stderr, "[c8] Failed to initialize C8!\n" );
				goto cleanup;
		}

		nuk_bg.r = 0.10f; nuk_bg.g = 0.18f; nuk_bg.b = 0.24f; nuk_bg.a = 1.0f;

		char *rom_name = NULL;
		int c, idx;
		opterr = 0;
		while ( (c = getopt(argc, argv, "r:")) != -1 ) {
				switch ( c ) {
						case 'r':
								rom_name = optarg;
						break;
						case '?':
								if ( optopt == 'r' ) {
										fprintf( stderr, "[c8] Option -%c requres an argument.\n", optopt );
								} else if ( isprint(optopt) ) {
										fprintf( stderr, "[c8] Unknown option `-%c'.\n", optopt );
								} else {
										fprintf( stderr, "[c8] Unknown option character `\\x%x'.\n", optopt );
										return EXIT_FAILURE;
								}
						break;
						default:
								abort();
				}

				for ( idx = optind; idx < argc; idx++ ) {
						printf( "Non-option argument %s\n", argv[idx] );
						return 0;
				}
		}


		//SDL_TimerID timer_id = SDL_AddTimer( 3 * 1000, PrintInfo, (void*)5 );

		if ( rom_name != NULL ) {
				C8LoadGame( rom_name );
				char new_title[512] = {'\0'};
				const char *old_title = SDL_GetWindowTitle( win );
				strncpy( new_title, old_title, strlen(old_title) );
				strncat( new_title, " - ", 1 );
				strncat( new_title, rom_name, strlen(rom_name) );
				SDL_SetWindowTitle( win, new_title );
		}

		SDL_StartTextInput();

		// Runtime variables
		bool quit = false;
		SDL_Event e;

		while ( !quit ) {
				nk_input_begin( nk_ctx );
				while ( SDL_PollEvent( &e ) != 0 ) {
						nk_sdl_handle_event( &e );

						// Requests
						if ( e.type == SDL_QUIT ) {
								quit = true;
								break; // Maybe premature?
						// Key presses
						} else if ( e.type == SDL_KEYDOWN ) {
								switch ( e.key.keysym.sym ) {
										case SDLK_ESCAPE:
												quit = true;
												break;
										case SDLK_UP:
												break;
										default:
												break;
								}
						} else if ( e.type == SDL_TEXTINPUT ) {
								int x = 0, y = 0;
								SDL_GetMouseState( &x, &y );
								HandleKeys( e.text.text[0], x, y );
						}
				}
				//nk_sdl_handle_grab();
				nk_input_end( nk_ctx );

				if ( nk_begin(nk_ctx, "Chipper Debug", nk_rect(50, 50, 220, 220),
										 NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
										 NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE) ) {

						char buf [2048];
						FormatString( buf, "Number of triangles: %d", renderer.triangle_count );
						nk_layout_row_dynamic( nk_ctx, 0, 1 );
						nk_layout_row_begin( nk_ctx, NK_STATIC, 0, 2 );
						{
								nk_layout_row_push( nk_ctx, 220 );
								nk_label( nk_ctx, buf, NK_TEXT_LEFT );
						}
						nk_layout_row_end( nk_ctx );

						//nk_text( nk_ctx, buf, sizeof(char) * strlen(buf), 1 );

						//nk_button_label(nk_ctx, "Button");
				}
				nk_end(nk_ctx);
        /*if (nk_begin(nk_ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(nk_ctx, 30, 80, 1);
            if (nk_button_label(nk_ctx, "button"))
                printf("button pressed!\n");
            nk_layout_row_dynamic(nk_ctx, 30, 2);
            if (nk_option_label(nk_ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(nk_ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(nk_ctx, 22, 1);
            nk_property_int(nk_ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(nk_ctx, 20, 1);
            nk_label(nk_ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(nk_ctx, 25, 1);
            if (nk_combo_begin_color(nk_ctx, nk_rgb_cf(nuk_bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(nk_ctx, 120, 1);
                nuk_bg = nk_color_picker(nk_ctx, nuk_bg, NK_RGBA);
                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nuk_bg.r = nk_propertyf(nk_ctx, "#R:", 0, nuk_bg.r, 1.0f, 0.01f,0.005f);
                nuk_bg.g = nk_propertyf(nk_ctx, "#G:", 0, nuk_bg.g, 1.0f, 0.01f,0.005f);
                nuk_bg.b = nk_propertyf(nk_ctx, "#B:", 0, nuk_bg.b, 1.0f, 0.01f,0.005f);
                nuk_bg.a = nk_propertyf(nk_ctx, "#A:", 0, nuk_bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(nk_ctx);
            }
        }
        nk_end(nk_ctx);*/

				Render();
				SDL_GL_SwapWindow( win );
		}

		SDL_StopTextInput();

cleanup:
		Log( LTRACE, "Cleaning up Doxie..." );
		SDL_GL_DeleteContext( gl_ctx );
		Doxie_RenderFree( &renderer );
		nk_sdl_shutdown();
		SDL_DestroyWindow( win );
		win = NULL;
		SDL_Quit();

		return 0;
}

