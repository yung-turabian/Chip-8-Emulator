/* Copyright 2025 Punchline Software

*/

#ifndef PUNCHLINE_FRAMEWORK_H
#define PUNCHLINE_FRAMEWORK_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <reis/base.h>
#include <reis/math.h>

#include "log.h"

#define MAX_TRIANGLES 20000 //2048 // Configurable
#define MAX_VERTICES MAX_TRIANGLES * 3

vec4 DoxBlack = (vec4) { 0.f, 0.f, 0.f, 1.f };
vec4 DoxWhite = (vec4) { 1.f, 1.f, 1.f, 1.f };
vec4 DoxRed = (vec4) { 1.f, 0.f, 0.f, 1.f };
vec4 DoxGreen = (vec4) { 0.f, 1.f, 0.f, 1.f };
vec4 DoxBlue = (vec4) { 0.f, 0.f, 1.f, 1.f };

typedef struct RenderVertex {
		vec2 pos;
		vec4 color;
} RenderVertex;

typedef struct DoxieRenderer {
		GLuint VAO;
		GLuint VBO;
		GLuint shader;

		// No IBO b/c the atom of the renderer is a triangle
		// A single triangle will never have any verticies shared.
		// If using quads, consider.

		mat4 projection;

		// Tightly packed triangle data. A CPU side mirror of the buffer.
		RenderVertex triangle_data[MAX_VERTICES];
		GLuint triangle_count;
} DoxieRenderer;


void PrintProgramLog( GLuint program );
void PrintShaderLog( GLuint shader );

bool Doxie_RenderInit( DoxieRenderer *r, unsigned int window_width, 
										   unsigned int window_height ) {
		bool success = true;

		glGenVertexArrays( 1, &r->VAO );
		glBindVertexArray( r->VAO );
		
		glGenBuffers( 1, &r->VBO );
		glBindBuffer( GL_ARRAY_BUFFER, r->VBO );
		glBufferData( GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(RenderVertex), NULL, GL_DYNAMIC_DRAW );

		glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 
										       (void*) offsetof(RenderVertex, pos) );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 
												   (void*) offsetof(RenderVertex, color) );
		glEnableVertexAttribArray( 1 );

		/*glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 
												   (void*) offsetof(RenderVertex, uv) );
		glEnableVertexAttribArray( 2 );
		glVertexAttribPointer( 3, 1, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), 
												   (void*) offsetof(RenderVertex, tex_index) );
		glEnableVertexAttribArray( 3 );*/

		// 1 : 1, origin at bottom left
		// Achieves this:
		// glViewport( 0, 0, window_width, window_height );
		r->projection = mat4_ortho( 0, window_width, window_height, 0, -0.01, 1.0 );

		r->shader = glCreateProgram();
		GLuint vert_module = glCreateShader( GL_VERTEX_SHADER );
		GLuint frag_module = glCreateShader( GL_FRAGMENT_SHADER );

		// TODO: file reading.

		const GLchar* vert_code = "#version 330 core\nlayout (location = 0) in vec2  a_pos; layout (location = 1) in vec4  a_color; out vec4 v_color; uniform mat4 u_proj; void main() { gl_Position = u_proj * vec4(a_pos, 0.0, 1.0); v_color = a_color; }";
		const GLchar* frag_code = "#version 330 core\nin vec4 v_color; layout (location=0) out vec4 f_color; void main() { f_color = v_color; }";

		//GLint vert_code_size = strlen( vert_code );
		glShaderSource( vert_module, 1, /*(const GLchar *const *)*/&vert_code, NULL );
		glCompileShader( vert_module );
		GLint vert_compiled = GL_FALSE;
		glGetShaderiv( vert_module, GL_COMPILE_STATUS, &vert_compiled );
		if ( vert_compiled != GL_TRUE ) {
				fprintf( stderr, "[ogl] Unable to compile vertex shader %d!\n", vert_module );
				PrintShaderLog( vert_module );
				success = false;
		}
		glAttachShader( r->shader, vert_module );
		Log( LTRACE, "Attached vertex shader" );


		//GLint frag_code_size = strlen( frag_code );
		glShaderSource( frag_module, 1, &frag_code, NULL );
		glCompileShader( frag_module );
		GLint frag_compiled = GL_FALSE;
		glGetShaderiv( frag_module, GL_COMPILE_STATUS, &frag_compiled );
		if ( frag_compiled != GL_TRUE ) {
				fprintf( stderr, "[ogl] Unable to compile fragment shader %d!\n", frag_module );
				PrintShaderLog( frag_module );
				//success = false;
		} 
		glAttachShader( r->shader, frag_module );
		Log( LTRACE, "Attached fragment shader" );

		glLinkProgram( r->shader );
		GLint program_success = GL_TRUE;
		glGetProgramiv( r->shader, GL_LINK_STATUS, &program_success );
		if ( program_success != GL_TRUE ) {
				fprintf( stderr, "[ogl] Error linking program %d!\n", r->shader );
				PrintProgramLog( r->shader );
				//success = false;
		}

		glDetachShader( r->shader, vert_module );
		glDeleteShader( vert_module );
		glDetachShader( r->shader, frag_module );
		glDeleteShader( frag_module );

		glClearColor( 0.f, 0.f, 0.f, 1.f );
		
		glUseProgram( r->shader );
		GLuint proj_loc = glGetUniformLocation( r->shader, "u_proj" );
		glUniformMatrix4fv( proj_loc, 1, GL_FALSE, r->projection.a );

		return success;
}


void Doxie_RenderFree( DoxieRenderer *r ) {
		glDeleteBuffers( 1, &r->VBO );
		glDeleteVertexArrays( 1, &r->VAO );
		glDeleteProgram( r->shader);
}


/**
 *	Reset triangle count from 0.
 */
void Doxie_RenderBeginFrame( DoxieRenderer *r ) {
		glClear( GL_COLOR_BUFFER_BIT );
		
		r->triangle_count = 0;
}


void Doxie_RenderEndFrame( DoxieRenderer *r ) {
		glUseProgram( r->shader );
		glBindVertexArray( r->VAO );
		glBindBuffer( GL_ARRAY_BUFFER, r->VBO );
		glBufferSubData( GL_ARRAY_BUFFER, 0, r->triangle_count * 3 * sizeof(RenderVertex), r->triangle_data );
		glDrawArrays( GL_TRIANGLES, 0, r->triangle_count * 3 );
}

// Does not account for textures.
void Doxie_RenderPushTriangle( DoxieRenderer *r, vec2 a, vec2 b, vec2 c,
														   vec4 a_color, vec4 b_color, vec4 c_color ) {
		// Flush the 'batch' if full, quite hacky and should opt for a solution
		// of using proper batches.
		if ( r->triangle_count == MAX_TRIANGLES ) {
				Doxie_RenderEndFrame( r );
				Doxie_RenderBeginFrame( r );
		}
		r->triangle_data[r->triangle_count * 3 + 0].pos = a;
		r->triangle_data[r->triangle_count * 3 + 0].color = a_color;
		r->triangle_data[r->triangle_count * 3 + 1].pos = b;
		r->triangle_data[r->triangle_count * 3 + 1].color = b_color;
		r->triangle_data[r->triangle_count * 3 + 2].pos = c;
		r->triangle_data[r->triangle_count * 3 + 2].color = c_color;

		r->triangle_count++;
}

// TODO: textures
// https://www.youtube.com/watch?v=NPnQF4yABwg&t=191s

// EXTENSIONS TO RENDERER

void Doxie_RenderPushQuadT( DoxieRenderer *r, rect quad, vec4 tint ) {
		Doxie_RenderPushTriangle( r,
														  $(vec2, quad.x, quad.y), 
															$(vec2, quad.x + quad.w, quad.y), 
															$(vec2, quad.x + quad.w, quad.y + quad.h),
															tint, tint, tint );
		Doxie_RenderPushTriangle( r,
														  $(vec2, quad.x, quad.y), 
															$(vec2, quad.x + quad.w, quad.y + quad.h), 
															$(vec2, quad.x, quad.y + quad.h),
															tint, tint, tint );
}

// Graphics program
/*GLuint gl_program_id = 0;
GLint gl_vertex_pos_2d_loc = -1;

GLuint VBO = 0;
GLuint IBO = 0;
GLuint VAO = 0; // Vertex Array Object*/

void PrintProgramLog( GLuint program ) {
		if ( glIsProgram(program) ) {
				int info_log_len = 0;
				int max_len = info_log_len;

				glGetProgramiv( program, GL_INFO_LOG_LENGTH, &max_len );
				char *info_log = (char*)calloc( max_len, sizeof(char) );

				glGetProgramInfoLog( program, max_len, &info_log_len, info_log );
				if ( info_log_len > 0 ) {
						printf( "%s\n", info_log );
				}

				free( info_log );
		} else {
				fprintf( stderr, "Name %d is not a program\n", program );
		}
}

void PrintShaderLog( GLuint shader ) {
		if ( glIsShader(shader) ) {
				int info_log_len = 0;
				int max_len = info_log_len;

				glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &max_len );
				char *info_log = (char*)calloc( max_len, sizeof(char) );

				glGetShaderInfoLog( shader, max_len, &info_log_len, info_log );
				if ( info_log_len > 0 ) {
						printf( "%s\n", info_log );
				}

				free( info_log );
		} else {
				fprintf( stderr, "Name %d is not a shader\n", shader );
		}
}

#endif /* PUNCHLINE_FRAMEWORK_H */
