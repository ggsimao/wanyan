#pragma once

#include <GL/glew.h>

template <GLenum target>
class Query {
public:
    Query();
    Query(const Query&) = delete;
    Query& operator=(const Query&) = delete;
    Query(Query&& other) noexcept;
    Query& operator=(Query&& other) noexcept;
    ~Query();
    GLuint id() const;
    bool available() const;
    // The measured value: nanoseconds, for elapsed-time queries. Only call
    // this once available() returns true; otherwise the CPU waits until the
    // GPU has finished the measured commands, stalling the pipeline.
    GLuint64 result() const;
private:
    void release() noexcept;
    GLuint _id = 0;
};

// Runs a query over the GL commands issued during its lifetime: begins it on
// construction, ends it on destruction. Queries of the same target can't
// nest: starting a second one while another is active is a GL error.
template <GLenum target>
class QueryScope {
public:
    explicit QueryScope(const Query<target>& query);
    QueryScope(const QueryScope&) = delete;
    QueryScope& operator=(const QueryScope&) = delete;
    QueryScope(QueryScope&&) = delete;
    QueryScope& operator=(QueryScope&&) = delete;
    ~QueryScope();
};

using TimeElapsedQuery = Query<GL_TIME_ELAPSED>;
using TimeElapsedQueryScope = QueryScope<GL_TIME_ELAPSED>;
