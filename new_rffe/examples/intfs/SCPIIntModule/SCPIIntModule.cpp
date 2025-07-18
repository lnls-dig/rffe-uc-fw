#include "SCPIIntModule.hpp"


SCPIIntModule::SCPIIntModule(const scpi_command_t * commands, void * ctx)
{
    static scpi_interface_t scpi_interface =
    {
        .error = SCPIIntModule::SCPI_Error,
        .write = SCPIIntModule::SCPI_Write,
        .control = nullptr,
        .flush = SCPIIntModule::SCPI_Flush,
        .reset = nullptr
    };

    SCPI_Init(&_scpi,
        commands,
        &scpi_interface,
        nullptr,
        nullptr, nullptr, nullptr, nullptr,
        _input_buffer, sizeof(_input_buffer),
        _error_queue, (sizeof(_error_queue)/sizeof(_error_queue[0]))
    );
    _holder.self = this;
    _holder.ctx = ctx;

    _scpi.user_context = &_holder;

    memset(_response_buffer, 0 , sizeof(_response_buffer));
    memset(_input_buffer, 0, sizeof(_input_buffer));
}

void SCPIIntModule::Process(const char *cmd) {
    memset(_response_buffer, 0, sizeof(_response_buffer));
    _response_buffer[0] = '\0';
    SCPI_Input(&_scpi, cmd, strlen(cmd));
}


size_t SCPIIntModule::SCPI_Write(scpi_t *ctx, const char *data, size_t len) {
    auto *holder = static_cast<SCPICtx*>(ctx->user_context);
    auto *self   = holder->self;

    if (!data || len == 0) return 0;

    const size_t cap = sizeof(self->_response_buffer);

    size_t off = std::strlen(self->_response_buffer);

    if (off >= cap - 1) return 0;

    const size_t avail    = (cap - 1) - off;
    const size_t copy_len = (len < avail) ? len : avail;

    std::memcpy(self->_response_buffer + off, data, copy_len);
    self->_response_buffer[off + copy_len] = '\0';

    return copy_len;
}

scpi_result_t SCPIIntModule::SCPI_Flush(scpi_t *ctx)
{
    auto *holder = static_cast<SCPICtx*>(ctx->user_context);
    auto *self   = holder->self;

    const size_t cap = sizeof(self->_response_buffer);
    size_t end = std::strlen(self->_response_buffer);
    if (cap) {
        if (end < cap) {
            self->_response_buffer[end] = '\0';
        } else {
            self->_response_buffer[cap - 1] = '\0';
        }
    }

    return SCPI_RES_OK;
}

int SCPIIntModule::SCPI_Error(scpi_t *ctx, int_fast16_t err)
{
    printf("SCPI **ERROR: %ld, \"%s\"\n", (int32_t) err, SCPI_ErrorTranslate(err));
    return 0;
}
