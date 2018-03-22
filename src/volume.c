#include <alsa/asoundlib.h>
#include <alsa/control.h>

#include "volume.h"
#include "desktop-utils/macros.h"

bool get_volume(long *vol) {
    bool ret = false;
    snd_hctl_t *hctl = NULL;
    snd_ctl_elem_id_t *id = NULL;
    snd_ctl_elem_value_t *control = NULL;
    snd_hctl_elem_t *elem = NULL;

    // To find card and subdevice: /proc/asound/, aplay -L, amixer controls
    CHK_NEG(snd_hctl_open(&hctl, "hw:0", 0));
    CHK_NEG(snd_hctl_load(hctl));

    snd_ctl_elem_id_alloca(&id);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);

    // amixer controls
    snd_ctl_elem_id_set_name(id, "Master Playback Volume");

    CHK_NULL(elem = snd_hctl_find_elem(hctl, id));

    snd_ctl_elem_value_alloca(&control);
    snd_ctl_elem_value_set_id(control, id);

    CHK_NEG(snd_hctl_elem_read(elem, control));
    *vol = snd_ctl_elem_value_get_integer(control,0);
    ret = true;

    fail:
    SAFE_FREE(snd_hctl_close, hctl);
    return ret;
}
