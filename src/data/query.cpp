#include "data/query.h"
#include <stdexcept>
#include <utility>

template <GLenum target>
Query<target>::Query() {
    glGenQueries(1, &_id);
    if (_id == 0) throw std::runtime_error("Couldn't make a query");
}

template <GLenum target>
Query<target>::Query(Query&& other) noexcept : _id(std::exchange(other._id, 0)) {}

template <GLenum target>
Query<target>& Query<target>::operator=(Query&& other) noexcept {
    if (this != &other) {
        release();
        _id = std::exchange(other._id, 0);
    }

    return *this;
}

template <GLenum target>
Query<target>::~Query() {
    release();
}

template <GLenum target>
void Query<target>::release() noexcept {
    if (_id != 0) {
        glDeleteQueries(1, &_id);
        _id = 0;
    }
}

template <GLenum target>
GLuint Query<target>::id() const {
    return _id;
}

template <GLenum target>
bool Query<target>::available() const {
    GLint value = GL_FALSE;
    glGetQueryObjectiv(_id, GL_QUERY_RESULT_AVAILABLE, &value);
    return value == GL_TRUE;
}

template <GLenum target>
GLuint64 Query<target>::result() const {
    GLuint64 value = 0;
    glGetQueryObjectui64v(_id, GL_QUERY_RESULT, &value);
    return value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <GLenum target>
QueryScope<target>::QueryScope(const Query<target>& query) {
    glBeginQuery(target, query.id());
}

template <GLenum target>
QueryScope<target>::~QueryScope() {
    glEndQuery(target);
}

template class Query<GL_TIME_ELAPSED>;
template class QueryScope<GL_TIME_ELAPSED>;
