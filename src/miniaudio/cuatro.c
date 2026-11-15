#include "math/math.h"
#include "files/fs.h"
#include "syscalls/syscalls.h"
#include "audio/cuatro.h"
#include "audio/mixer.h"
#include "audio/wav.h"
#include "audio/tone.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#define MOG_LEVEL MOG_VERBOSE
#define MOG_MODULE "Audio"
#include "utils/mog.h"

#define MA_FORMAT ma_format_s16

static file mixer = { };

ma_device audio_dev = {};
ma_sound audio_sound = {};//TODO: prob don't need
ma_engine audio_engine = {};

bool audio_init_done = false;
bool audio_avail = false;

ma_audio_buffer ma_buf = {};

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, u32 frameCount){
    ma_audio_buffer_read_pcm_frames(pDevice->pUserData, pOutput, frameCount, 0);
}

static inline bool init_audio_device(){
    if (audio_init_done) return audio_avail;
    ma_device_config dconfig = ma_device_config_init(ma_device_type_playback);
    dconfig.playback.format = MA_FORMAT;
    dconfig.playback.channels = 1;
    dconfig.sampleRate = 44100;
    dconfig.pUserData = &ma_buf;
    dconfig.dataCallback = data_callback;
    
    audio_init_done = true;
    
    if (ma_device_init(0, &dconfig, &audio_dev) != MA_SUCCESS){
        mog(ERROR,"Failed to initialize device");
        return false;
    }
    
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &audio_dev;
    
    if (ma_engine_init(&engineConfig, &audio_engine) != MA_SUCCESS){
        mog(ERROR,"Failed to initialize engine");
        return false;
    }
    
    mog(VERBOSE,"Audio initialized");
    
    audio_avail = true;
    
    return true;
}

bool mixer_open_file(){
    init_audio_device();
    return (mixer.id > 0) || (openf("/audio/output", &mixer) == FS_RESULT_SUCCESS);
}

int8_t mixer_open_line(){
    mixer_line_data data;
    if (mixer_open_file()){
        if (mixer_read_line(-1, &data)) return data.lineId;
    }
    return -1;
}

void mixer_close_line(int8_t lineId){
    if (mixer_open_file() && lineId >= 0){
        mixer_command command = { .lineId = lineId, .command = MIXER_CLOSE_LINE };
        writef(&mixer, (char*)&command, sizeof(mixer_command));
    }
}

bool mixer_read_line(int8_t lineId, mixer_line_data* data){
    if (mixer_open_file()){
        data->lineId = lineId;
        return (sizeof(mixer_line_data) == readf(&mixer, (char*)data, sizeof(mixer_line_data)));
    }
    return false;
}

bool mixer_still_playing(int8_t lineId){
    if (mixer_open_file()){
        mixer_line_data data;
        data.lineId = lineId;
        if (mixer_read_line(lineId, &data)){
            if (data.dbl.buf[0].ptr != 0 || data.dbl.buf[1].ptr != 0) return true;
        }
    }
    return false;
}

bool mixer_mute(){
    if (mixer_open_file()){
        mixer_command command = { .lineId = -1, .command = MIXER_MUTE };
        writef(&mixer, (char*)&command, sizeof(mixer_command));
    }
    return false; // TODO: return prev setting
}

bool mixer_unmute(){
    if (mixer_open_file()){
        mixer_command command = { .lineId = -1, .command = MIXER_UNMUTE };
        writef(&mixer, (char*)&command, sizeof(mixer_command));
    }
    return true; // TODO: return prev setting
}

int16_t mixer_master_level(int16_t level){
    if (mixer_open_file()){
        ma_device_set_master_volume(&audio_dev, (float)level/AUDIO_LEVEL_MAX);
    }
    return level; // TODO: return prev setting
}

int16_t mixer_line_level(int8_t lineId, int16_t level, int16_t pan){
    if (lineId < 0 || lineId > MIXER_INPUTS) return 0;
    if (mixer_open_file()){
        mixer_command command = { .lineId = lineId, .command = MIXER_SETLEVEL, .level = level, .pan = pan };
        writef(&mixer, (char*)&command, sizeof(mixer_command));
    }
    return level; // TODO: return prev setting
}

static bool mixer_play_async(int8_t lineId, audio_samples* audio, uint32_t delay_ms, AUDIO_LIFETIME life, int16_t level, int16_t pan){
    ma_audio_buffer_config config = ma_audio_buffer_config_init(
        MA_FORMAT,
        audio->channels,
        audio->smpls_per_channel * audio->channels,
        (void*)audio->samples.ptr,
        0);
    
    ma_result result = ma_audio_buffer_init(&config, &ma_buf);
    if (result != MA_SUCCESS) {
        mog(ERROR,"Failed to init audio buffer %i",result);
        return false;
    }
    
    result = ma_sound_init_from_data_source(&audio_engine, &ma_buf, 0, 0, &audio_sound);
    if (result != MA_SUCCESS) {
        mog(ERROR,"Failed to init sound %i",result);
        return false;
    }
    
    return true;
    // mixer_command command = { .lineId = lineId, .command = MIXER_PLAY, .audio = audio, .delay_ms = delay_ms, .life = life, .level = level, .pan = pan };
    // return (sizeof(mixer_command) == writef(&mixer, (char*)&command, sizeof(mixer_command)));
}

bool audio_play_sync(audio_samples *audio, uint32_t delay_ms, AUDIO_LIFETIME life, int16_t level, int16_t pan){
    int8_t lineId = mixer_open_line();
    if (lineId < 0 || false == mixer_play_async(lineId, audio, delay_ms, life, level, pan)) return false;
    do {
        msleep(5);
    } while (mixer_still_playing(lineId));
    mixer_close_line(lineId);
    return true;
}

int8_t audio_play_async(audio_samples *audio, uint32_t delay_ms, AUDIO_LIFETIME life, int16_t level, int16_t pan){
    int8_t lineId = 1;//mixer_open_line();
    // if (lineId >= 0) {
        if (false == mixer_play_async(lineId, audio, delay_ms, life, level, pan)) {
            mixer_close_line(lineId);
            lineId = -1;
        }
    // }
    return lineId;
}

bool audio_update_stream(int8_t lineId, sizedptr samples){
    if (mixer_open_file()){
        mixer_command command = { .lineId = lineId, .command = MIXER_SETBUFFER, .samples = samples };
        return (sizeof(mixer_command) == writef(&mixer, (char*)&command, sizeof(mixer_command)));
    }
    return false;
}