
// #include <engine_server/Server.h>
// #include <iostream>
// #define THREADS 1

// template <typename PlayerType>
// Server<PlayerType>::~Server() {
//     std::cout<<"Closing.\n";
//     sock.close();
//     inp_sock.close();
//     context->close();
// }

// template <typename PlayerType>
// void Server<PlayerType>::Initialize() {
//     context = new zmq::context_t(THREADS);
//     sock = zmq::socket_t(*context, zmq::socket_type::pub);
//     sock.bind(endpoint+":"+port);

//     inp_sock = zmq::socket_t(*context, zmq::socket_type::rep);
//     inp_sock.bind(endpoint+":"+inp_port);
// }

// template <typename PlayerType>
// void Server<PlayerType>::RecvInputOrConnectionPackets(EntityManager* em) {
   
//         zmq::message_t req(sizeof(rr_packet));
        
//         rr_packet* p = (rr_packet*)req.data();
//         p->packet_type = -2;
//         inp_sock.recv(req, zmq::recv_flags::dontwait);

//         if (p->packet_type != -2) {
//             rr_packet* p_request = (rr_packet*)req.data();

//             if (p_request->packet_type == P_CLIENT_HELLO) {
                
//                 PlayerType* newplayer = new PlayerType(*SpawnPrototypeOnJoin);
//                 if (dynamic_cast<PlayerEntity*>(newplayer)) {
//                     // std::cout<<(int)p_request->client_id<<"\n";
//                     newplayer->client_id = p_request->client_id;
//                     newplayer->SetNumKeys(p_request->numkeys);

//                     em->AddEntity(newplayer);
//                     // std::cout<<"A\n";
//                     playerVector.push_back(newplayer);

//                     numConnectedPlayers = playerVector.size();
//                 }
//                 // std::cout<<"B\n";
//                 // send map to the client

//                 zmq::message_t rep(sizeof(rr_packet));
//                 rr_packet* rp = (rr_packet*)rep.data();
//                 rp->packet_type = loaded_map;
//                 inp_sock.send(rep, zmq::send_flags::dontwait);
                
//                 // send other connected clients

//                 // 
//             }

//             if (p_request->packet_type == P_CLIENT_INPUT) {
                
            
//                 // queue up input packets
//                 rr_packet pushb;
//                 memcpy(&pushb, p_request, sizeof(rr_packet));
//                 inpPackets.push_back(pushb);
//                 // std::cout<<(int)pushb.keycode<<" "<<(int)pushb.keystate<<"\n";

//                 if (inp_sock.get(zmq::sockopt::rcvmore)) {
                    
//                     bool recvmore = true;

//                     while (recvmore) {
//                         // std::cout<<"recv_keys\n";
//                         zmq::message_t nmsg = zmq::message_t(sizeof(rr_packet));
//                         inp_sock.recv(nmsg, zmq::recv_flags::none);

//                         if (!inp_sock.get(zmq::sockopt::rcvmore)) {
//                             recvmore = false;
//                             continue;
//                         }

//                         rr_packet pushb;
//                         memcpy(&pushb, p_request, sizeof(rr_packet));
//                         inpPackets.push_back(pushb);
//                     }
//                 }
                
//                 zmq::message_t rep = zmq::message_t(sizeof(rr_packet));
//                 rr_packet* p = (rr_packet*)rep.data();
//                 p->packet_type = 0;
//                 inp_sock.send(rep, zmq::send_flags::none);
//             }
//         }
// }

// template <typename PlayerType>
// void Server<PlayerType>::PublishUpdates(EntityManager* em) {
   
//         std::vector<Entity*>& evref = em->getEntityVectorRef();
//         for (int i = 0; i < evref.size(); i++) {
//             zmq::message_t em_packet = zmq::message_t(sizeof(ps_packet));
//             ps_packet* data = (ps_packet*)em_packet.data();

//             *data = evref[i]->Packetize(P_ENTITY_UPDATE_FETCH);
//             data->entity_id = evref[i]->entity_id;
//             sock.send(em_packet, zmq::send_flags::sndmore);
            
//         }
        
//         zmq::message_t end_packet = zmq::message_t(sizeof(ps_packet));
//         ps_packet* data = (ps_packet*)end_packet.data();
//         data->packet_type = P_ENTITY_UPDATE_FETCH;
//         sock.send(end_packet, zmq::send_flags::none);
    
// }

