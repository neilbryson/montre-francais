#include <tizen.h>
#include "montrefrancais.h"
#include "francais.h"
#include <dlog.h>

typedef struct appdata {
    Evas_Object *win;
    Evas_Object *conform;
    Evas_Object *layer_heure;
    Evas_Object *layer_heure_ou_minute;
    Evas_Object *layer_minute;
    Evas_Object *layer_date;
    Evas_Object *layer_batterie;
    Evas_Object *layer_pas;
} appdata_s;

#define TEXT_BUF_SIZE 64
#define MAX_NUM_SIZE 10

static int needs_espace(int nombre) {
    return (nombre == 21 || nombre == 31 || nombre == 41 || nombre == 51);
}

static int needs_tiret(int nombre) {
    return (
        (nombre > 16 && nombre < 20 && nombre != 21)
        || (nombre > 21 && nombre < 30)
        || (nombre > 31 && nombre < 40)
        || (nombre > 41 && nombre < 50)
        || nombre > 51
    );
}

static char ponctuation(int nombre) {
    if (needs_espace(nombre)) {
        return ' ';
    }

    if (needs_tiret(nombre)) {
        return '-';
    }

    return '\0';
}

static void montre_heure(appdata_s *ad, int heure) {
    char texte[TEXT_BUF_SIZE];
    char texte_heure[MAX_NUM_SIZE];
    char texte_heure_2[MAX_NUM_SIZE];
    char ponct = ponctuation(heure);
    char h[7];

    if (heure == 0) {
        strcpy(texte_heure, "minuit");
    } else if (heure == 12) {
        strcpy(texte_heure, "midi");
    } else {
        strcpy(texte_heure, nombre[heure][0]);
        strcpy(texte_heure_2, nombre[heure][1]);
    }

    if (heure == 1) {
        strcpy(h, "heure");
    } else if (heure != 0 && heure != 12) {
        strcpy(h, "heures");
    }

    dlog_print(DLOG_DEBUG, LOG_TAG, "Texte heure %s", texte_heure);
    snprintf(texte, TEXT_BUF_SIZE, "<align=center>il est<br/><b>%s%c%s</b><br/>%s</align>", texte_heure, ponct, texte_heure_2, h);
    elm_object_text_set(ad->layer_heure, texte);
}

static void montre_minute(appdata_s *ad, int minute) {
    char texte[TEXT_BUF_SIZE];
    char texte_minute[MAX_NUM_SIZE];
    char texte_minute_2[MAX_NUM_SIZE];

    strcpy(texte_minute, nombre[minute][0]);
    strcpy(texte_minute_2, nombre[minute][1]);

    char ponct = ponctuation(minute);
    snprintf(texte, TEXT_BUF_SIZE, "<align=center>%s%c%s</align>", texte_minute, ponct, texte_minute_2);
    elm_object_text_set(ad->layer_minute, texte);
}

static void update_watch(appdata_s *ad, watch_time_h watch_time, int ambient) {
    int heure;
    int minute;
    int jour;
    int jourSemaine;
    int leMois;


    if (watch_time == NULL)
        return;

    watch_time_get_hour24(watch_time, &heure);
    watch_time_get_minute(watch_time, &minute);
    watch_time_get_day(watch_time, &jour);
    watch_time_get_day_of_week(watch_time, &jourSemaine);
    watch_time_get_month(watch_time, &leMois);

    dlog_print(DLOG_DEBUG, LOG_TAG, "DEB %d:%d", heure, minute);

    montre_heure(ad, heure);
    montre_minute(ad, minute);
}

static void create_base_gui(appdata_s *ad, int width, int height) {
    int ret;
    watch_time_h watch_time = NULL;

    /* Window */
    ret = watch_app_get_elm_win(&ad->win);
    if (ret != APP_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
        return;
    }

    evas_object_resize(ad->win, width, height);

    /* Conformant */
    ad->conform = elm_conformant_add(ad->win);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(ad->win, ad->conform);
    evas_object_show(ad->conform);

    Evas_Object *arriere_plan = evas_object_rectangle_add(ad->win);
    evas_object_move(arriere_plan, 0, 0);
    evas_object_resize(arriere_plan, width, height);
    evas_object_show(arriere_plan);

    ad->layer_heure = elm_label_add(arriere_plan);
    evas_object_resize(ad->layer_heure, width, height / 3);
    evas_object_move(ad->layer_heure, 0, height / 8);
    evas_object_show(ad->layer_heure);

    ad->layer_minute = elm_label_add(arriere_plan);
    evas_object_resize(ad->layer_minute, width, height / 6);
    evas_object_move(ad->layer_minute, 0, (height / 2) - (height / 16));
    evas_object_show(ad->layer_minute);

    ret = watch_time_get_current_time(&watch_time);
    if (ret != APP_ERROR_NONE)
        dlog_print(DLOG_ERROR, LOG_TAG, "failed to get current time. err = %d", ret);

    update_watch(ad, watch_time, 0);
    watch_time_delete(watch_time);

    /* Show window after base gui is set up */
    evas_object_show(ad->win);
}

static bool app_create(int width, int height, void *data) {
    /* Hook to take necessary actions before main event loop starts
        Initialize UI resources and application's data
        If this function returns true, the main loop of application starts
        If this function returns false, the application is terminated */
    appdata_s *ad = data;

    create_base_gui(ad, width, height);

    return true;
}

static void app_control(app_control_h app_control, void *data) {
    /* Handle the launch request. */
}

static void app_pause(void *data) {
    /* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data) {
    /* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data) {
    /* Release all resources. */
}

static void app_time_tick(watch_time_h watch_time, void *data) {
    /* Called at each second while your app is visible. Update watch UI. */
    appdata_s *ad = data;
    update_watch(ad, watch_time, 0);
}

static void app_ambient_tick(watch_time_h watch_time, void *data) {
    /* Called at each minute while the device is in ambient mode. Update watch UI. */
    appdata_s *ad = data;
    update_watch(ad, watch_time, 1);
}

static void app_ambient_changed(bool ambient_mode, void *data) {
    /* Update your watch UI to conform to the ambient mode */
}

static void watch_app_lang_changed(app_event_info_h event_info, void *user_data) {
    /*APP_EVENT_LANGUAGE_CHANGED*/
    char *locale = NULL;
    app_event_get_language(event_info, &locale);
    elm_language_set(locale);
    free(locale);
    return;
}

static void watch_app_region_changed(app_event_info_h event_info, void *user_data) {
    /*APP_EVENT_REGION_FORMAT_CHANGED*/
}

int main(int argc, char *argv[]) {
    appdata_s ad = {0,};
    int ret = 0;

    watch_app_lifecycle_callback_s event_callback = {0,};
    app_event_handler_h handlers[5] = {NULL, };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;
    event_callback.time_tick = app_time_tick;
    event_callback.ambient_tick = app_ambient_tick;
    event_callback.ambient_changed = app_ambient_changed;

    watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
        APP_EVENT_LANGUAGE_CHANGED, watch_app_lang_changed, &ad);
    watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
        APP_EVENT_REGION_FORMAT_CHANGED, watch_app_region_changed, &ad);

    ret = watch_app_main(argc, argv, &event_callback, &ad);
    if (ret != APP_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
    }

    return ret;
}

