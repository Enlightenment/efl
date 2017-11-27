#ifndef SGDEBUG_H
#define SGDEBUG_H
#include <cstdint>
#include <memory>
#include <string>
#include <iosfwd>
#include <type_traits>



    enum class LogLevel : uint8_t { INFO, WARN, CRIT, OFF };


class SGDebug
{
public:
    SGDebug();
    SGDebug& debug() {return *this;}
    SGDebug(LogLevel level, char const * file, char const * function, uint32_t line);
    ~SGDebug();

    SGDebug(SGDebug &&) = default;
    SGDebug& operator=(SGDebug &&) = default;

    void stringify(std::ostream & os);

    SGDebug& operator<<(char arg);
    SGDebug& operator<<(int32_t arg);
    SGDebug& operator<<(uint32_t arg);
    //SGDebug& operator<<(int64_t arg);
    //SGDebug& operator<<(uint64_t arg);

    SGDebug& operator<<(long arg);
    SGDebug& operator<<(unsigned long arg);
    SGDebug& operator<<(double arg);
    SGDebug& operator<<(std::string const & arg);

    template < size_t N >
    SGDebug& operator<<(const char (&arg)[N]) {
        encode(string_literal_t(arg));
        return *this;
    }

    template < typename Arg >
    typename std::enable_if < std::is_same < Arg, char const * >::value, SGDebug& >::type
    operator<<(Arg const & arg) {
        encode(arg);
        return *this;
    }

    template < typename Arg >
    typename std::enable_if < std::is_same < Arg, char * >::value, SGDebug& >::type
    operator<<(Arg const & arg) {
        encode(arg);
        return *this;
    }

    struct string_literal_t {
        explicit string_literal_t(char const * s) : m_s(s) {}
        char const * m_s;
    };

private:
    char * buffer();

    template < typename Arg >
    void encode(Arg arg);

    template < typename Arg >
    void encode(Arg arg, uint8_t type_id);

    void encode(char * arg);
    void encode(char const * arg);
    void encode(string_literal_t arg);
    void encode_c_string(char const * arg, size_t length);
    void resize_buffer_if_needed(size_t additional_bytes);
    void stringify(std::ostream & os, char * start, char const * const end);

private:
    size_t m_bytes_used;
    size_t m_buffer_size;
    std::unique_ptr < char [] > m_heap_buffer;
    bool m_logAll;
    char m_stack_buffer[256 - sizeof(bool) - 2 * sizeof(size_t) - sizeof(decltype(m_heap_buffer)) - 8 /* Reserved */];
};

struct SGDebugServer
{
    /*
     * Ideally this should have been operator+=
     * Could not get that to compile, so here we are...
     */
    bool operator==(SGDebug &);
};

void set_log_level(LogLevel level);

bool is_logged(LogLevel level);


/*
 * Non guaranteed logging. Uses a ring buffer to hold log lines.
 * When the ring gets full, the previous log line in the slot will be dropped.
 * Does not block producer even if the ring buffer is full.
 * ring_buffer_size_mb - LogLines are pushed into a mpsc ring buffer whose size
 * is determined by this parameter. Since each LogLine is 256 bytes,
 * ring_buffer_size = ring_buffer_size_mb * 1024 * 1024 / 256
 */
struct NonGuaranteedLogger
{
    NonGuaranteedLogger(uint32_t ring_buffer_size_mb_) : ring_buffer_size_mb(ring_buffer_size_mb_) {}
    uint32_t ring_buffer_size_mb;
};

/*
 * Provides a guarantee log lines will not be dropped.
 */
struct GuaranteedLogger
{
};

/*
 * Ensure initialize() is called prior to any log statements.
 * log_directory - where to create the logs. For example - "/tmp/"
 * log_file_name - root of the file name. For example - "nanolog"
 * This will create log files of the form -
 * /tmp/nanolog.1.txt
 * /tmp/nanolog.2.txt
 * etc.
 * log_file_roll_size_mb - mega bytes after which we roll to next log file.
 */
void initialize(GuaranteedLogger gl, std::string const & log_directory,
                std::string const & log_file_name, uint32_t log_file_roll_size_mb);
void initialize(NonGuaranteedLogger ngl, std::string const & log_directory,
                std::string const & log_file_name, uint32_t log_file_roll_size_mb);


#define SGDEBUG_LOG(LEVEL) SGDebugServer() == SGDebug(LEVEL, __FILE__, __func__, __LINE__).debug()
#define sgDebug is_logged(LogLevel::INFO) && SGDEBUG_LOG(LogLevel::INFO)
#define sgWarning is_logged(LogLevel::INFO) && SGDEBUG_LOG(LogLevel::WARN)
#define sgCritical is_logged(LogLevel::INFO) && SGDEBUG_LOG(LogLevel::CRIT)

#endif // SGDEBUG_H
