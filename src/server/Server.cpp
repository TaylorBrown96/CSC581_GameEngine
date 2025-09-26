
#include <server/Server.h>

#define THREADS 1

void Server::Initialize() {
    context = new zmq::context_t(THREADS);
    sock = zmq::socket_t(*context, zmq::socket_type::pub);
    sock.bind(endpoint+":"+port);

    inp_sock = zmq::socket_t(*context, zmq::socket_type::rep);
    inp_sock.bind(endpoint+":"+inp_port);
}


void Server::RecvInputOrConnectionPackets() {
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // std::cout<<"Started input connection:\n";
    // while (true) {
        zmq::message_t req(sizeof(rr_packet));
        
        rr_packet* p = (rr_packet*)req.data();
        p->packet_type = -2;
        inp_sock.recv(req, zmq::recv_flags::dontwait);
        if (p->packet_type != 2) {
            rr_packet* p_request = (rr_packet*)req.data();

            if (p_request->packet_type == P_CLIENT_HELLO) {
                // std::cout<<"Client connected\n";
                zmq::message_t rep(sizeof(rr_packet));
                rr_packet* rp = (rr_packet*)rep.data();
                rp->packet_type = loaded_map;
                inp_sock.send(rep, zmq::send_flags::dontwait);
                // send map type so that they could load all the same objects loaded here

            }

            if (p_request->packet_type == P_CLIENT_INPUT) {

                // apply input engine side
            }
        }
}

void Server::PublishUpdates(EntityManager* em) {
    // std::this_thread::sleep_for(std::chrono::seconds(5));
    // std::cout<<"Started Update connection:\n";
    // while (true) {
        std::vector<Entity*>& evref = em->getEntityVectorRef();
        for (int i = 0; i < evref.size(); i++) {
            zmq::message_t em_packet = zmq::message_t(sizeof(ps_packet));
            ps_packet* data = (ps_packet*)em_packet.data();

            *data = evref[i]->Packetize(P_ENTITY_UPDATE_FETCH);
            data->entity_id = evref[i]->entity_id;
            sock.send(em_packet, zmq::send_flags::sndmore);
            // std::cout<<"SendingUpdate "<<(int)i<<"\n";
        }
        
        zmq::message_t end_packet = zmq::message_t(sizeof(ps_packet));
        ps_packet* data = (ps_packet*)end_packet.data();
        data->packet_type = P_ENTITY_UPDATE_FETCH;
        sock.send(end_packet, zmq::send_flags::none);
        // std::cout<<"EndUpdate\n";
    

}

