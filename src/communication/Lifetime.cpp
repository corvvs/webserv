#include "Lifetime.hpp"
#include "../utils/test_common.hpp"

#define MILLISECOND_IN_MS 1
#define SECOND_IN_MS 1000
#define MINUTE_IN_MS 60000

Lifetime::Lifetime(const t_time_epoch_ms &active, const t_time_epoch_ms &inactive)
    : lifetime_active(active), lifetime_inactive(inactive) {
    deactivated_at = WSTime::get_epoch_ms();
    activated_at   = 0;
}

bool Lifetime::is_active() const {
    return activated_at >= deactivated_at;
}

void Lifetime::activate() {
    if (!is_active()) {
        activated_at = WSTime::get_epoch_ms();
    }
}

void Lifetime::deactivate() {
    if (is_active()) {
        deactivated_at = WSTime::get_epoch_ms();
    }
}

bool Lifetime::is_timeout(t_time_epoch_ms now) const {
    if (is_active()) {
        // アクティブ状態
        const bool is_mortal           = lifetime_active > 0;
        const bool is_actually_timeout = now > activated_at && now - activated_at > lifetime_active;
        if (is_mortal && is_actually_timeout) {
            return true;
        }
    } else {
        // 非アクティブ状態
        const bool is_mortal           = lifetime_inactive > 0;
        const bool is_actually_timeout = now > deactivated_at && now - deactivated_at > lifetime_inactive;
        if (is_mortal && is_actually_timeout) {
            return true;
        }
    }

    return false;
}

const t_time_epoch_ms Lifetime::LIFETIME_CONNECTION_ACTIVE   = 0;
const t_time_epoch_ms Lifetime::LIFETIME_CONNECTION_INACTIVE = 5 * SECOND_IN_MS;
Lifetime Lifetime::make_connection() {
    return Lifetime(LIFETIME_CONNECTION_ACTIVE, LIFETIME_CONNECTION_INACTIVE);
}

const t_time_epoch_ms Lifetime::LIFETIME_ROUNDTRIP_ACTIVE   = 60 * SECOND_IN_MS;
const t_time_epoch_ms Lifetime::LIFETIME_ROUNDTRIP_INACTIVE = 60 * SECOND_IN_MS;
Lifetime Lifetime::make_round_trip() {
    return Lifetime(LIFETIME_ROUNDTRIP_ACTIVE, LIFETIME_ROUNDTRIP_INACTIVE);
}

const t_time_epoch_ms Lifetime::LIFETIME_REQUEST_ACTIVE   = 30 * SECOND_IN_MS;
const t_time_epoch_ms Lifetime::LIFETIME_REQUEST_INACTIVE = 0;
Lifetime Lifetime::make_request() {
    return Lifetime(LIFETIME_REQUEST_ACTIVE, LIFETIME_REQUEST_INACTIVE);
}

const t_time_epoch_ms Lifetime::LIFETIME_REQUEST_HEADER_ACTIVE   = 15 * SECOND_IN_MS;
const t_time_epoch_ms Lifetime::LIFETIME_REQUEST_HEADER_INACTIVE = 0;
Lifetime Lifetime::make_request_header() {
    return Lifetime(LIFETIME_REQUEST_HEADER_ACTIVE, LIFETIME_REQUEST_HEADER_INACTIVE);
}

const t_time_epoch_ms Lifetime::LIFETIME_RESPONSE_ACTIVE   = 30 * SECOND_IN_MS;
const t_time_epoch_ms Lifetime::LIFETIME_RESPONSE_INACTIVE = 0;
Lifetime Lifetime::make_response() {
    return Lifetime(LIFETIME_RESPONSE_ACTIVE, LIFETIME_RESPONSE_INACTIVE);
}

const t_time_epoch_ms Lifetime::LIFETIME_CGI_ACTIVE   = 60 * SECOND_IN_MS;
const t_time_epoch_ms Lifetime::LIFETIME_CGI_INACTIVE = 0;
Lifetime Lifetime::make_cgi() {
    return Lifetime(LIFETIME_CGI_ACTIVE, LIFETIME_CGI_INACTIVE);
}
