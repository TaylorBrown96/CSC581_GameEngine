
#include <zmq.hpp>

#include <packetdef.h>
#include <GameEngine.h>
#include <iostream>
#include <engine_client/Client.h>

Client::Client() : Entity() {
    SetOverseer();
}

void Client::initClient(std::string p_endpoint, std::string port, std::string inp_port) {
    con = zmq::context_t(1);
    sock = zmq::socket_t(con, zmq::socket_type::req);
    upd_sock = zmq::socket_t(con, zmq::socket_type::sub);
    update_endpoint = p_endpoint + ":" + port;
    input_endpoint = p_endpoint + ":" + inp_port;

}

void Client::ConnectInit(int* map_type) {
    sock.connect(input_endpoint);

    rr_packet rp;
    rp.packet_type = P_CLIENT_HELLO;
    
    sendRRPacket(&rp);

    rr_packet rep;
    RecvRRPacket(&rep);
    *map_type = rep.packet_type;

    upd_sock.connect(update_endpoint);
    char topic = P_ENTITY_UPDATE_FETCH;
    upd_sock.setsockopt(ZMQ_SUBSCRIBE, &topic, sizeof(char));
}

void Client::sendRRPacket(rr_packet* pack) {
    zmq::message_t msg = zmq::message_t(sizeof(rr_packet));
    memcpy(msg.data(), pack, sizeof(rr_packet));
    sock.send(msg, zmq::send_flags::none);
}

void Client::RecvRRPacket(rr_packet* pack) {
    zmq::message_t msg = zmq::message_t(sizeof(rr_packet));
    sock.recv(msg, zmq::recv_flags::none);
    memcpy(pack, msg.data(), sizeof(rr_packet));
}

void Client::QueueUpdatePackets() {
    
    while (true) {
        
        zmq::message_t nmsg = zmq::message_t(sizeof(ps_packet));
        upd_sock.recv(nmsg, zmq::recv_flags::none);
        
        if (!upd_sock.get(zmq::sockopt::rcvmore))
            break;

        ps_packet ps;

        memcpy(&ps, nmsg.data(), sizeof(ps_packet));
        
        if (ps.packet_type != P_STREAM_DONE)
            packQueue.push_back(ps);    

        
    }
}

void Client::ApplyQueueUpdatePacketsToState(EntityManager* em) {
    std::vector<Entity*>& evref = em->getEntityVectorRef();

    while (packQueue.size() > 0) {
        // std::cout<<"Applying\n";
        ps_packet pack_top = packQueue.back();
        bool found = false;
        for (int i = 0; i < evref.size() - 1 && !found; i++) {
            if (evref[i]->entity_id == pack_top.entity_id) {
                evref[i]->Unpacketize(&pack_top);
                found = true;
            }
        }

        packQueue.pop_back();
    }

}

void Client::QueueInputs(InputManager* in) {
    // this sends key presses in the form of key updates
    // i.e. if the W key is just pressed it will send a packet with the W scancode and status K_KEYDOWN
    
    int nk;
    
    char* kdiff = in->GetKeyDiff(&nk);

    bool ready_for_recv = false;

    for (int n = 0; n < nk; n++) {
       
        if (kdiff[n] != 0) {
            ready_for_recv = true;
            
            rr_packet keypacket;
            keypacket.packet_type = P_CLIENT_INPUT;
            keypacket.keycode = n;
            if (kdiff[n] == -1) {
                keypacket.keystate = K_KEYUP;
            }
            else if (kdiff[n] == 1) {
                keypacket.keystate = K_KEYDOWN;
            }
            inpPackets.push_back(keypacket);
        }
    }
    
    for (int k = 0; k < inpPackets.size(); k++) {
        zmq::message_t msg = zmq::message_t(sizeof(rr_packet));
        memcpy(msg.data(), &inpPackets[k], sizeof(rr_packet));
        zmq::send_flags flg = zmq::send_flags::sndmore;
        if (k == inpPackets.size() - 1)
            flg = zmq::send_flags::none;
        sock.send(msg, flg);
    }
    
    inpPackets.clear();

    if (ready_for_recv) {
        zmq::message_t msgr = zmq::message_t(sizeof(rr_packet));
        sock.recv(msgr, zmq::recv_flags::none);
    }
}

void Client::Update(float dt, InputManager* in, EntityManager* em) {
    // Queue up inputs from the client and send to server (should move into separate thread? unlikely tbh)
    QueueInputs(in);

    // Queue up packets from server
    QueueUpdatePackets();

    // apply packets gathered from server
    ApplyQueueUpdatePacketsToState(em);
}
