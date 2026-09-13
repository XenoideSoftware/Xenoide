#pragma once

#include "glaze/producer/RegistryLoader.h"

#include <string_view>

namespace glaze::testfixtures {

// Minimal OpenGL registry XML used by the producer and generator test suites.
// This is a verbatim port of conan/packages/glaze/tests/conftest.py's
// MINI_XML fixture so the C++ tests cover the same shapes as the Python ones.
inline constexpr std::string_view kMiniXml = R"XML(<?xml version="1.0" encoding="UTF-8"?>
<registry>
  <types>
    <type>typedef unsigned int <name>GLuint</name>;</type>
    <type>typedef unsigned int <name>GLenum</name>;</type>
    <type>typedef int <name>GLint</name>;</type>
    <type>typedef int <name>GLsizei</name>;</type>
    <type>typedef unsigned int <name>GLbitfield</name>;</type>
    <type>typedef void <name>GLvoid</name>;</type>
    <type>typedef unsigned char <name>GLubyte</name>;</type>
    <type>typedef char <name>GLchar</name>;</type>
    <type>typedef ptrdiff_t <name>GLsizeiptr</name>;</type>
  </types>

  <enums namespace="GL" group="BufferTargetARB" type="enum">
    <enum value="0x8892" name="GL_ARRAY_BUFFER" group="BufferTargetARB"/>
    <enum value="0x8893" name="GL_ELEMENT_ARRAY_BUFFER" group="BufferTargetARB"/>
  </enums>

  <enums namespace="GL" group="ClearBufferMask" type="bitmask">
    <enum value="0x00004000" name="GL_COLOR_BUFFER_BIT" group="ClearBufferMask"/>
    <enum value="0x00000100" name="GL_DEPTH_BUFFER_BIT" group="ClearBufferMask"/>
  </enums>

  <enums namespace="GL">
    <enum value="0" name="GL_FALSE"/>
    <enum value="1" name="GL_TRUE"/>
    <enum value="0x1406" name="GL_FLOAT"/>
  </enums>

  <enums namespace="GL" group="QueryTarget" type="enum">
    <enum value="0x8914" name="GL_SAMPLES_PASSED" group="QueryTarget"/>
    <enum value="0x8C2F" name="GL_ANY_SAMPLES_PASSED" group="QueryTarget"/>
  </enums>

  <commands namespace="GL">
    <command>
      <proto>void <name>glClear</name></proto>
      <param group="ClearBufferMask"><ptype>GLbitfield</ptype> <name>mask</name></param>
    </command>
    <command>
      <proto>void <name>glBindBuffer</name></proto>
      <param group="BufferTargetARB"><ptype>GLenum</ptype> <name>target</name></param>
      <param class="buffer"><ptype>GLuint</ptype> <name>buffer</name></param>
    </command>
    <command>
      <proto>const <ptype>GLubyte</ptype> *<name>glGetString</name></proto>
      <param><ptype>GLenum</ptype> <name>name</name></param>
    </command>
    <command>
      <proto>void <name>glGenBuffers</name></proto>
      <param><ptype>GLsizei</ptype> <name>n</name></param>
      <param class="buffer"><ptype>GLuint</ptype> *<name>buffers</name></param>
    </command>
    <command>
      <proto>void <name>glDeleteBuffers</name></proto>
      <param><ptype>GLsizei</ptype> <name>n</name></param>
      <param class="buffer">const <ptype>GLuint</ptype> *<name>buffers</name></param>
    </command>
    <command>
      <proto><ptype>GLuint</ptype> <name>glCreateProgram</name></proto>
    </command>
    <command>
      <proto>void <name>glDeleteProgram</name></proto>
      <param class="program"><ptype>GLuint</ptype> <name>program</name></param>
    </command>
    <command>
      <proto><ptype>GLuint</ptype> <name>glCreateShader</name></proto>
      <param><ptype>GLenum</ptype> <name>type</name></param>
    </command>
    <command>
      <proto>void <name>glDeleteShader</name></proto>
      <param class="shader"><ptype>GLuint</ptype> <name>shader</name></param>
    </command>
    <command>
      <proto>void <name>glLinkProgram</name></proto>
      <param class="program"><ptype>GLuint</ptype> <name>program</name></param>
    </command>
    <command>
      <proto><ptype>GLint</ptype> <name>glGetUniformLocation</name></proto>
      <param class="program"><ptype>GLuint</ptype> <name>program</name></param>
      <param>const <ptype>GLchar</ptype> *<name>name</name></param>
    </command>
    <command>
      <proto>void <name>glAttachShader</name></proto>
      <param class="program"><ptype>GLuint</ptype> <name>program</name></param>
      <param class="shader"><ptype>GLuint</ptype> <name>shader</name></param>
    </command>
    <command>
      <proto>void <name>glNamedBufferData</name></proto>
      <param class="buffer"><ptype>GLuint</ptype> <name>buffer</name></param>
      <param><ptype>GLsizei</ptype> <name>size</name></param>
      <param len="size">const void *<name>data</name></param>
      <param group="BufferUsageARB"><ptype>GLenum</ptype> <name>usage</name></param>
    </command>
    <command>
      <proto>void <name>glNamedBufferSubData</name></proto>
      <param class="buffer"><ptype>GLuint</ptype> <name>buffer</name></param>
      <param><ptype>GLsizei</ptype> <name>offset</name></param>
      <param><ptype>GLsizei</ptype> <name>size</name></param>
      <param>const void *<name>data</name></param>
    </command>
    <command>
      <proto>void <name>glCreateQueries</name></proto>
      <param group="QueryTarget"><ptype>GLenum</ptype> <name>target</name></param>
      <param><ptype>GLsizei</ptype> <name>n</name></param>
      <param class="query"><ptype>GLuint</ptype> *<name>ids</name></param>
    </command>
    <command>
      <proto>void <name>glDeleteQueries</name></proto>
      <param><ptype>GLsizei</ptype> <name>n</name></param>
      <param class="query">const <ptype>GLuint</ptype> *<name>ids</name></param>
    </command>
    <command>
      <proto>void <name>glDrawArraysInstancedARB</name></proto>
      <param><ptype>GLenum</ptype> <name>mode</name></param>
      <param><ptype>GLint</ptype> <name>first</name></param>
      <param><ptype>GLsizei</ptype> <name>count</name></param>
      <param><ptype>GLsizei</ptype> <name>primcount</name></param>
    </command>
    <command>
      <proto>void <name>glBufferData</name></proto>
      <param group="BufferTargetARB"><ptype>GLenum</ptype> <name>target</name></param>
      <param><ptype>GLsizeiptr</ptype> <name>size</name></param>
      <param len="size">const void *<name>data</name></param>
      <param group="BufferUsageARB"><ptype>GLenum</ptype> <name>usage</name></param>
    </command>
    <command>
      <proto>void <name>glGetShaderiv</name></proto>
      <param class="shader"><ptype>GLuint</ptype> <name>shader</name></param>
      <param><ptype>GLenum</ptype> <name>pname</name></param>
      <param><ptype>GLint</ptype> *<name>params</name></param>
    </command>
    <command>
      <proto>void <name>glGetShaderInfoLog</name></proto>
      <param class="shader"><ptype>GLuint</ptype> <name>shader</name></param>
      <param><ptype>GLsizei</ptype> <name>bufSize</name></param>
      <param len="1"><ptype>GLsizei</ptype> *<name>length</name></param>
      <param len="bufSize"><ptype>GLchar</ptype> *<name>infoLog</name></param>
    </command>
  </commands>

  <feature api="gl" name="GL_VERSION_1_0" number="1.0">
    <require>
      <enum name="GL_COLOR_BUFFER_BIT"/>
      <enum name="GL_DEPTH_BUFFER_BIT"/>
      <enum name="GL_FALSE"/>
      <enum name="GL_TRUE"/>
      <enum name="GL_FLOAT"/>
      <command name="glClear"/>
      <command name="glGetString"/>
    </require>
  </feature>

  <feature api="gl" name="GL_VERSION_1_5" number="1.5">
    <require>
      <enum name="GL_ARRAY_BUFFER"/>
      <enum name="GL_ELEMENT_ARRAY_BUFFER"/>
      <command name="glBindBuffer"/>
      <command name="glGenBuffers"/>
      <command name="glDeleteBuffers"/>
      <command name="glBufferData"/>
    </require>
  </feature>

  <feature api="gl" name="GL_VERSION_2_0" number="2.0">
    <require>
      <command name="glCreateProgram"/>
      <command name="glDeleteProgram"/>
      <command name="glCreateShader"/>
      <command name="glDeleteShader"/>
      <command name="glLinkProgram"/>
      <command name="glGetUniformLocation"/>
      <command name="glAttachShader"/>
      <command name="glGetShaderiv"/>
      <command name="glGetShaderInfoLog"/>
    </require>
  </feature>

  <feature api="gl" name="GL_VERSION_3_1" number="3.1">
    <require>
    </require>
    <remove profile="core">
      <enum name="GL_FLOAT"/>
    </remove>
  </feature>

  <feature api="gl" name="GL_VERSION_4_5" number="4.5">
    <require>
      <enum name="GL_SAMPLES_PASSED"/>
      <enum name="GL_ANY_SAMPLES_PASSED"/>
      <command name="glNamedBufferData"/>
      <command name="glNamedBufferSubData"/>
      <command name="glCreateQueries"/>
      <command name="glDeleteQueries"/>
    </require>
  </feature>

  <feature api="gles2" name="GL_ES_VERSION_2_0" number="2.0">
    <require>
      <enum name="GL_COLOR_BUFFER_BIT"/>
      <command name="glClear"/>
    </require>
  </feature>

  <commands namespace="GL">
    <command>
      <proto>void <name>glMakeBufferResidentNV</name></proto>
      <param group="BufferTargetARB"><ptype>GLenum</ptype> <name>target</name></param>
      <param><ptype>GLenum</ptype> <name>access</name></param>
    </command>
    <command>
      <proto>void <name>glDebugMessageCallbackKHR</name></proto>
      <param><ptype>GLenum</ptype> <name>source</name></param>
      <param><ptype>GLuint</ptype> <name>id</name></param>
    </command>
  </commands>

  <extensions>
    <extension name="GL_ARB_buffer_storage" supported="gl|gles2">
      <require>
        <enum name="GL_ARRAY_BUFFER"/>
      </require>
    </extension>
    <extension name="GL_ARB_draw_instanced" supported="gl">
      <require>
        <command name="glDrawArraysInstancedARB"/>
      </require>
    </extension>
    <extension name="GL_NV_shader_buffer_load" supported="gl">
      <require>
        <command name="glMakeBufferResidentNV"/>
      </require>
    </extension>
    <extension name="GL_KHR_debug" supported="gl|gles2">
      <require>
        <command name="glDebugMessageCallbackKHR"/>
      </require>
    </extension>
  </extensions>
</registry>
)XML";

/// Load the MINI_XML fixture into a Registry via the producer.
inline glaze::model::Registry loadMiniRegistry() {
    return producer::RegistryLoader{}.loadFromString(kMiniXml);
}

} // namespace glaze::testfixtures
