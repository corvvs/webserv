#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "../config/Config.hpp"
#include "../interface/IObserver.hpp"
#include "../interface/IRouter.hpp"
#include "../interface/ISocketLike.hpp"
#include "../socket/SocketListening.hpp"
#include "../utils/FileCacher.hpp"
#include <map>
#include <utility>

// [チャネルクラス]
// [責務]
// - リスニングソケット1つを保持すること; ISocketLike
// - 接続要求に応じてコネクションクラスを生成して返すこと; acceptするということ
// - TODO: 同じプロトコル/ポートを持つバーチャルホスト(server)をまとめて管理すること
class Channel : public ISocketLike {
public:
    // Channelを識別するためのID
    // プロトコルファミリーとポートの組
    typedef std::pair<t_socket_domain, t_port> t_channel_id;

    struct Attribute {
        SocketListening *sock;
        IRouter *router;
        const config::config_vector &configs;
        FileCacher &cacher;

        Attribute(SocketListening *sock, IRouter *router, const config::config_vector &configs, FileCacher &cacher);
        ~Attribute();
    };

private:
    Attribute attr;

public:
    Channel(IRouter *router,
            t_socket_domain sdomain,
            t_socket_type stype,
            t_port port,
            const config::config_vector &configs,
            FileCacher &cacher);
    ~Channel();

    t_fd get_fd() const;
    t_port get_port() const;
    void notify(IObserver &observer, IObserver::observation_category cat, t_time_epoch_ms epoch);

    t_channel_id get_id() const;
};

#endif
