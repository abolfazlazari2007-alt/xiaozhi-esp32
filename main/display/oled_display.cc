#include "oled_display.h"
#include "assets/lang_config.h"
#include "lvgl_font.h"
#include "lvgl_theme.h"
#include "settings.h"

#include <algorithm>
#include <cstring>
#include <string>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <material_symbols.h>
#include <noto_emoji.h>

#define TAG "OledDisplay"

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);
LV_FONT_DECLARE(BUILTIN_ICON_FONT);
LV_FONT_DECLARE(font_material_symbols_30_1);
LV_FONT_DECLARE(font_noto_emoji_30_1);

namespace {

// مختصات چهره برای صفحهٔ 128x64 طراحی شده‌اند.
// هر جزء، یک شکل سفید روی پس‌زمینهٔ سیاه است.
enum class RobotFace {
    Neutral,
    Happy,
    Laughing,
    Sad,
    Angry,
    Crying,
    Loving,
    Surprised,
    Shocked,
    Winking,
    Cool,
    Sleepy,
    Thinking,
    Relaxed,
    Confused,
    Silly,
    Embarrassed,
    Kissy,
    Confident
};

void Bar(lv_obj_t* parent, int x, int y, int w, int h,
         int radius = 0, bool white = true) {
    if (parent == nullptr || w <= 0 || h <= 0) {
        return;
    }

    lv_obj_t* shape = lv_obj_create(parent);
    lv_obj_remove_style_all(shape);
    lv_obj_set_pos(shape, x, y);
    lv_obj_set_size(shape, w, h);
    lv_obj_set_style_bg_color(
        shape, white ? lv_color_white() : lv_color_black(), 0);
    lv_obj_set_style_bg_opa(shape, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(shape, radius, 0);
}

void NormalEyes(lv_obj_t* root) {
    Bar(root, 29, 20, 18, 8, 4);
    Bar(root, 81, 20, 18, 8, 4);
}

void ClosedEyes(lv_obj_t* root) {
    Bar(root, 26, 25, 23, 4, 2);
    Bar(root, 79, 25, 23, 4, 2);
}

void HappyEye(lv_obj_t* root, int x, int y) {
    // قوس چشم خندان، بدون قاب یا دایرهٔ دور صورت
    Bar(root, x,      y + 11, 5, 5, 2);
    Bar(root, x + 4,  y + 6,  5, 6, 2);
    Bar(root, x + 8,  y + 3,  7, 5, 2);
    Bar(root, x + 14, y + 6,  5, 6, 2);
    Bar(root, x + 18, y + 11, 5, 5, 2);
}

void HappyEyes(lv_obj_t* root) {
    HappyEye(root, 26, 12);
    HappyEye(root, 79, 12);
}

void SmallSmile(lv_obj_t* root) {
    Bar(root, 49, 38, 5, 4, 2);
    Bar(root, 53, 41, 6, 4, 2);
    Bar(root, 58, 43, 12, 4, 2);
    Bar(root, 69, 41, 6, 4, 2);
    Bar(root, 74, 38, 5, 4, 2);
}

void WideSmile(lv_obj_t* root) {
    // دهان بزرگ و توپر، نزدیک به صورت خندانِ عکس
    Bar(root, 43, 34, 42, 4, 2);
    Bar(root, 44, 37, 40, 4, 2);
    Bar(root, 46, 40, 36, 4, 2);
    Bar(root, 49, 43, 30, 4, 2);
    Bar(root, 53, 46, 22, 4, 2);
    Bar(root, 58, 49, 12, 3, 2);
}

void Frown(lv_obj_t* root) {
    Bar(root, 49, 46, 5, 4, 2);
    Bar(root, 53, 42, 6, 4, 2);
    Bar(root, 58, 40, 12, 4, 2);
    Bar(root, 69, 42, 6, 4, 2);
    Bar(root, 74, 46, 5, 4, 2);
}

void OpenMouth(lv_obj_t* root) {
    Bar(root, 55, 35, 18, 21, 9);
    Bar(root, 59, 39, 10, 13, 5, false);
}

void HeartEye(lv_obj_t* root, int x) {
    Bar(root, x + 1, 17, 8, 7, 4);
    Bar(root, x + 11, 17, 8, 7, 4);
    Bar(root, x, 22, 20, 4, 2);
    Bar(root, x + 2, 25, 16, 4, 2);
    Bar(root, x + 5, 28, 10, 4, 2);
    Bar(root, x + 8, 31, 4, 3, 2);
}

void CrossEye(lv_obj_t* root, int x) {
    for (int i = 0; i < 5; ++i) {
        Bar(root, x + i * 4, 17 + i * 3, 5, 4, 1);
        Bar(root, x + i * 4, 29 - i * 3, 5, 4, 1);
    }
}

void AngryEyes(lv_obj_t* root) {
    Bar(root, 31, 25, 15, 6, 3);
    Bar(root, 82, 25, 15, 6, 3);

    Bar(root, 25, 17, 8, 3, 1);
    Bar(root, 31, 19, 8, 3, 1);
    Bar(root, 37, 21, 9, 3, 1);

    Bar(root, 82, 21, 9, 3, 1);
    Bar(root, 89, 19, 8, 3, 1);
    Bar(root, 95, 17, 8, 3, 1);
}

void SadEyes(lv_obj_t* root) {
    Bar(root, 29, 23, 17, 7, 3);
    Bar(root, 82, 23, 17, 7, 3);
    Bar(root, 28, 19, 7, 3, 1);
    Bar(root, 34, 20, 7, 3, 1);
    Bar(root, 87, 20, 7, 3, 1);
    Bar(root, 93, 19, 7, 3, 1);
}

RobotFace GetRobotFace(const char* emotion, bool& recognized) {
    recognized = true;

    if (emotion == nullptr || std::strcmp(emotion, "neutral") == 0 ||
        std::strcmp(emotion, "robot_2") == 0) {
        return RobotFace::Neutral;
    }
    if (std::strcmp(emotion, "happy") == 0 ||
        std::strcmp(emotion, "delicious") == 0) {
        return RobotFace::Happy;
    }
    if (std::strcmp(emotion, "laughing") == 0 ||
        std::strcmp(emotion, "funny") == 0) {
        return RobotFace::Laughing;
    }
    if (std::strcmp(emotion, "sad") == 0) {
        return RobotFace::Sad;
    }
    if (std::strcmp(emotion, "angry") == 0) {
        return RobotFace::Angry;
    }
    if (std::strcmp(emotion, "crying") == 0) {
        return RobotFace::Crying;
    }
    if (std::strcmp(emotion, "loving") == 0 ||
        std::strcmp(emotion, "heart_eyes") == 0) {
        return RobotFace::Loving;
    }
    if (std::strcmp(emotion, "surprised") == 0) {
        return RobotFace::Surprised;
    }
    if (std::strcmp(emotion, "shocked") == 0) {
        return RobotFace::Shocked;
    }
    if (std::strcmp(emotion, "winking") == 0) {
        return RobotFace::Winking;
    }
    if (std::strcmp(emotion, "cool") == 0 ||
        std::strcmp(emotion, "sunglasses") == 0) {
        return RobotFace::Cool;
    }
    if (std::strcmp(emotion, "sleepy") == 0) {
        return RobotFace::Sleepy;
    }
    if (std::strcmp(emotion, "thinking") == 0) {
        return RobotFace::Thinking;
    }
    if (std::strcmp(emotion, "relaxed") == 0) {
        return RobotFace::Relaxed;
    }
    if (std::strcmp(emotion, "confused") == 0) {
        return RobotFace::Confused;
    }
    if (std::strcmp(emotion, "silly") == 0) {
        return RobotFace::Silly;
    }
    if (std::strcmp(emotion, "embarrassed") == 0) {
        return RobotFace::Embarrassed;
    }
    if (std::strcmp(emotion, "kissy") == 0) {
        return RobotFace::Kissy;
    }
    if (std::strcmp(emotion, "confident") == 0) {
        return RobotFace::Confident;
    }

    recognized = false;
    return RobotFace::Neutral;
}

void DrawRobotFace(lv_obj_t* root, RobotFace face) {
    if (root == nullptr) {
        return;
    }

    lv_obj_clean(root);

    switch (face) {
        case RobotFace::Neutral:
            NormalEyes(root);
            Bar(root, 53, 44, 22, 4, 2);
            break;

        case RobotFace::Happy:
        case RobotFace::Laughing:
            HappyEyes(root);
            WideSmile(root);
            break;

        case RobotFace::Sad:
            SadEyes(root);
            Frown(root);
            break;

        case RobotFace::Angry:
            AngryEyes(root);
            Bar(root, 51, 44, 26, 4, 2);
            break;

        case RobotFace::Crying:
            SadEyes(root);
            Bar(root, 41, 32, 4, 12, 2);
            Bar(root, 94, 32, 4, 12, 2);
            Bar(root, 40, 42, 6, 5, 3);
            Bar(root, 93, 42, 6, 5, 3);
            Frown(root);
            break;

        case RobotFace::Loving:
            HeartEye(root, 26);
            HeartEye(root, 82);
            SmallSmile(root);
            break;

        case RobotFace::Surprised:
            Bar(root, 30, 17, 15, 16, 7);
            Bar(root, 83, 17, 15, 16, 7);
            OpenMouth(root);
            break;

        case RobotFace::Shocked:
            CrossEye(root, 27);
            CrossEye(root, 80);
            OpenMouth(root);
            break;

        case RobotFace::Winking:
            Bar(root, 26, 25, 24, 4, 2);
            HappyEye(root, 79, 12);
            SmallSmile(root);
            break;

        case RobotFace::Cool:
            // قاب عینک؛ قسمت سیاه، شیشهٔ عینک است.
            Bar(root, 22, 17, 32, 19, 6);
            Bar(root, 25, 20, 26, 13, 4, false);
            Bar(root, 74, 17, 32, 19, 6);
            Bar(root, 77, 20, 26, 13, 4, false);
            Bar(root, 53, 21, 22, 4, 2);
            SmallSmile(root);
            break;

        case RobotFace::Sleepy:
            ClosedEyes(root);
            Bar(root, 58, 42, 12, 4, 2);
            break;

        case RobotFace::Thinking:
            Bar(root, 27, 23, 21, 5, 2);
            Bar(root, 83, 20, 16, 8, 4);
            Bar(root, 59, 43, 20, 4, 2);
            break;

        case RobotFace::Relaxed:
            ClosedEyes(root);
            SmallSmile(root);
            break;

        case RobotFace::Confused:
            Bar(root, 29, 20, 18, 8, 4);
            Bar(root, 83, 24, 18, 5, 2);
            Bar(root, 53, 44, 22, 4, 2);
            break;

        case RobotFace::Silly:
            Bar(root, 26, 25, 24, 4, 2);
            Bar(root, 82, 20, 18, 8, 4);
            WideSmile(root);
            Bar(root, 61, 48, 7, 9, 3);
            break;

        case RobotFace::Embarrassed:
            NormalEyes(root);
            Bar(root, 20, 35, 10, 3, 1);
            Bar(root, 98, 35, 10, 3, 1);
            SmallSmile(root);
            break;

        case RobotFace::Kissy:
            HappyEyes(root);
            Bar(root, 59, 39, 10, 10, 5);
            Bar(root, 62, 42, 4, 4, 2, false);
            break;

        case RobotFace::Confident:
            Bar(root, 28, 21, 20, 6, 3);
            Bar(root, 81, 21, 20, 6, 3);
            SmallSmile(root);
            break;
    }
}

}  // namespace

OledDisplay::OledDisplay(esp_lcd_panel_io_handle_t panel_io,
                         esp_lcd_panel_handle_t panel,
                         int width, int height,
                         bool mirror_x, bool mirror_y)
    : panel_io_(panel_io), panel_(panel) {
    width_ = width;
    height_ = height;

    auto text_font = std::make_shared<LvglBuiltInFont>(&BUILTIN_TEXT_FONT);
    auto icon_font = std::make_shared<LvglBuiltInFont>(&BUILTIN_ICON_FONT);
    auto large_icon_font =
        std::make_shared<LvglBuiltInFont>(&font_material_symbols_30_1);
    auto emoji_font =
        std::make_shared<LvglBuiltInFont>(&font_noto_emoji_30_1);

    auto dark_theme = new LvglTheme("dark");
    dark_theme->set_text_font(text_font);
    dark_theme->set_icon_font(icon_font);
    dark_theme->set_large_icon_font(large_icon_font);
    dark_theme->set_emoji_font(emoji_font);

    auto& theme_manager = LvglThemeManager::GetInstance();
    theme_manager.RegisterTheme("dark", dark_theme);
    current_theme_ = dark_theme;

    ESP_LOGI(TAG, "Initialize LVGL");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    port_cfg.task_stack = 6144;
#if CONFIG_SOC_CPU_CORES_NUM > 1
    port_cfg.task_affinity = 1;
#endif
    lvgl_port_init(&port_cfg);

    ESP_LOGI(TAG, "Adding OLED display");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .control_handle = nullptr,
        .buffer_size = static_cast<uint32_t>(width_ * height_),
        .double_buffer = false,
        .trans_size = 0,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = mirror_x,
            .mirror_y = mirror_y,
        },
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .full_refresh = 0,
            .direct_mode = 0,
        },
    };

    display_ = lvgl_port_add_disp(&display_cfg);
    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }
}

void OledDisplay::SetupUI() {
    if (setup_ui_called_) {
        ESP_LOGW(TAG, "SetupUI() called multiple times, skipping duplicate call");
        return;
    }

    Display::SetupUI();
    if (height_ == 64) {
        SetupUI_128x64();
    } else {
        SetupUI_128x32();
    }
}

OledDisplay::~OledDisplay() {
    if (content_ != nullptr) {
        lv_obj_del(content_);
    }

    bool is_128x64_layout = (top_bar_ != nullptr);
    if (status_bar_ != nullptr && is_128x64_layout) {
        status_label_ = nullptr;
        notification_label_ = nullptr;
        lv_obj_del(status_bar_);
    }
    if (top_bar_ != nullptr) {
        network_label_ = nullptr;
        mute_label_ = nullptr;
        battery_label_ = nullptr;
        lv_obj_del(top_bar_);
    }
    if (side_bar_ != nullptr) {
        if (!is_128x64_layout) {
            status_label_ = nullptr;
            notification_label_ = nullptr;
            network_label_ = nullptr;
            mute_label_ = nullptr;
            battery_label_ = nullptr;
        }
        lv_obj_del(side_bar_);
    }
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }

    if (panel_ != nullptr) {
        esp_lcd_panel_del(panel_);
    }
    if (panel_io_ != nullptr) {
        esp_lcd_panel_io_del(panel_io_);
    }
    lvgl_port_deinit();
}

bool OledDisplay::Lock(int timeout_ms) {
    return lvgl_port_lock(timeout_ms);
}

void OledDisplay::Unlock() {
    lvgl_port_unlock();
}

void OledDisplay::SetChatMessage(const char* role, const char* content) {
    // نمایش متن گفتگو عمداً خاموش است: صفحه فقط چهره را نشان می‌دهد.
}

void OledDisplay::RemoveChatMessage(const char* role, const char* content) {
    // در حالت نمایش چهره، پیام گفت‌وگو وجود ندارد.
}

void OledDisplay::SetupUI_128x64() {
    DisplayLockGuard lock(this);

    auto screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);

    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, width_, height_);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_radius(container_, 0, 0);
    lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(container_);

    // برای سازگاری با کد وضعیتِ موجود ساخته و پنهان می‌شوند.
    top_bar_ = lv_obj_create(container_);
    lv_obj_add_flag(top_bar_, LV_OBJ_FLAG_HIDDEN);

    status_bar_ = lv_obj_create(screen);
    lv_obj_add_flag(status_bar_, LV_OBJ_FLAG_HIDDEN);
    status_label_ = lv_label_create(status_bar_);
    notification_label_ = lv_label_create(status_bar_);

    // از همین عضوِ موجود، این بار به‌عنوان ظرفِ شکل‌های چهره استفاده می‌کنیم.
    emotion_label_ = lv_obj_create(container_);
    lv_obj_remove_style_all(emotion_label_);
    lv_obj_set_size(emotion_label_, 128, 64);
    lv_obj_set_style_pad_all(emotion_label_, 0, 0);
    lv_obj_set_style_bg_opa(emotion_label_, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(emotion_label_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(emotion_label_);
    DrawRobotFace(emotion_label_, RobotFace::Neutral);

    content_ = nullptr;
    content_left_ = nullptr;
    content_right_ = nullptr;
    chat_message_label_ = nullptr;
}

void OledDisplay::SetupUI_128x32() {
    DisplayLockGuard lock(this);

    auto lvgl_theme = static_cast<LvglTheme*>(current_theme_);
    auto text_font = lvgl_theme->text_font()->font();
    auto icon_font = lvgl_theme->icon_font()->font();
    auto large_icon_font = lvgl_theme->large_icon_font()->font();

    auto screen = lv_screen_active();
    lv_obj_set_style_text_font(screen, text_font, 0);

    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);

    content_ = lv_obj_create(container_);
    lv_obj_set_size(content_, 32, 32);
    lv_obj_set_style_pad_all(content_, 0, 0);
    lv_obj_set_style_border_width(content_, 0, 0);
    lv_obj_set_style_radius(content_, 0, 0);

    emotion_label_ = lv_label_create(content_);
    lv_obj_set_style_text_font(emotion_label_, large_icon_font, 0);
    lv_label_set_text(emotion_label_, MATERIAL_SYMBOLS_ROBOT_2);
    lv_obj_center(emotion_label_);

    side_bar_ = lv_obj_create(container_);
    lv_obj_set_size(side_bar_, width_ - 32, 32);
    lv_obj_set_flex_flow(side_bar_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(side_bar_, 0, 0);
    lv_obj_set_style_border_width(side_bar_, 0, 0);
    lv_obj_set_style_radius(side_bar_, 0, 0);
    lv_obj_set_style_pad_row(side_bar_, 0, 0);

    status_bar_ = lv_obj_create(side_bar_);
    lv_obj_set_size(status_bar_, width_ - 32, 16);
    lv_obj_set_style_radius(status_bar_, 0, 0);
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_pad_column(status_bar_, 0, 0);

    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_obj_set_style_pad_left(status_label_, 2, 0);
    lv_label_set_text(status_label_, Lang::Strings::INITIALIZING);

    notification_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_pad_left(notification_label_, 2, 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, icon_font, 0);

    network_label_ = lv_label_create(status_bar_);
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, icon_font, 0);

    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, icon_font, 0);

    chat_message_label_ = lv_label_create(side_bar_);
    lv_obj_set_size(chat_message_label_, width_ - 32, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_left(chat_message_label_, 2, 0);
    lv_label_set_long_mode(chat_message_label_,
                           LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text(chat_message_label_, "");

    static lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_delay(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_obj_set_style_anim(chat_message_label_, &a, LV_PART_MAIN);
    lv_obj_set_style_anim_duration(
        chat_message_label_,
        lv_anim_speed_clamped(60, 300, 60000),
        LV_PART_MAIN);
}

bool OledDisplay::SetEmotion(const char* emotion) {
    if (height_ == 64) {
        bool recognized = false;
        RobotFace face = GetRobotFace(emotion, recognized);

        DisplayLockGuard lock(this);
        if (emotion_label_ == nullptr) {
            return false;
        }

        DrawRobotFace(emotion_label_, face);
        return recognized;
    }

    // مسیر قدیمی نمایشگر 128x32 دست‌نخورده باقی مانده است.
    auto lvgl_theme = static_cast<LvglTheme*>(current_theme_);
    const char* utf8 =
        emotion ? noto_emoji_get_utf8(emotion) : nullptr;
    const lv_font_t* emotion_font =
        lvgl_theme->emoji_font()->font();

    if (utf8 == nullptr && emotion != nullptr) {
        utf8 = material_symbols_get_utf8(emotion);
        emotion_font = lvgl_theme->large_icon_font()->font();
    }

    DisplayLockGuard lock(this);
    if (emotion_label_ == nullptr) {
        return false;
    }

    if (utf8 != nullptr) {
        lv_obj_set_style_text_font(emotion_label_, emotion_font, 0);
        lv_label_set_text(emotion_label_, utf8);
        return true;
    }

    lv_obj_set_style_text_font(
        emotion_label_, lvgl_theme->emoji_font()->font(), 0);
    lv_label_set_text(emotion_label_, NOTO_EMOJI_NEUTRAL);
    return false;
}

void OledDisplay::SetTheme(Theme* theme) {
    DisplayLockGuard lock(this);
    auto lvgl_theme = static_cast<LvglTheme*>(theme);
    auto text_font = lvgl_theme->text_font()->font();
    lv_obj_set_style_text_font(lv_screen_active(), text_font, 0);
}

void OledDisplay::SetPowerSaveMode(bool on) {
    if (panel_) {
        Settings settings("wifi", false);
        if (settings.GetBool("power_save_display_off", false)) {
            esp_lcd_panel_disp_on_off(panel_, !on);
        }
    }
    LvglDisplay::SetPowerSaveMode(on);
}
