#ifndef _vencoder__hh
#define _vencoder__hh

/** FIXME: vc_open 将需要增加很多入口参数, 用于设置编码属性 */
void *vencoder_open (int width, int height, double fps);
int vencoder_close (void *ctx);

/** 每当 返回 > 0, 说明得到一帧数据, 此时可以调用 get_last_frame_info() 获取更多信息 */
int vencoder (void *ctx, unsigned char *data[4], int stride[4], const void **buf, int *len);
int get_last_frame_info (void *ctx, int *key_frame, int64_t *pts, int64_t *dts);

/** 强制尽快输出关键帧 */
int ve_force_keyframe (void *ctx);
x264_param_t *get_x264_param_t(void* ctx);
x264_t *get_x264_handle(void* ctx);
#endif //

