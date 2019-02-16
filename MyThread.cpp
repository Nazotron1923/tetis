//
// Created by nazar on 2/12/19.
//

#include "MyThread.h"



void *MyThread::Entry(){

    sf::Thread* thread = 0;

    if(this->th_iter == 0){ //server
        bool done = true;

        if (listener.listen(PORT) == Socket::Done)
        {
            cout << "Server success" << endl;
        }
        else
            exit(0);

        selector.add(listener);

        while (done)
        {
            if (selector.wait())
            {
                if (selector.isReady(listener))
                {
                    TcpSocket *socket = new TcpSocket();
                    listener.accept(*socket);
                    selector.add(*socket);
                    Clients.push_back(socket);

                    Packet receivePacket;

                }

                else
                {
                    for (int i = 0;i < (int)Clients.size();i++)
                    {
                        if (selector.isReady(*Clients[i]))
                        {
                            Packet receivePacket;
                            if (Clients[i]->receive(receivePacket) == Socket::Done)
                            {
                                string pseudo;
                                short int message;
                                receivePacket >> pseudo >> msgSend;
                                cout << "Received message. Client " << pseudo << " : " << msgSend << endl;

                                Packet sendPacket;
                                sendPacket << pseudo << msgSend;
                                /*for (int j = 0;j < (int)Clients.size();j++)
                                {
                                    cout << "Send for client " <<j<<" ("<<pseudo << "): " << msgSend << endl;
                                    Clients[j]->send(sendPacket);
                                }*/
                            }
                        }
                    }
                }
            }
            wxMilliSleep(2000);
        }

    }else{ //client

        bool done = true;
        string pseudo;

        cout << "Name: ";
        cin >> pseudo;

        if (socket.connect(IPADDRESS, PORT) == Socket::Done)
        {
            cout << "Connection accepted" << endl;
            socket.setBlocking(false);
            Packet sendPacket;
            sendPacket << pseudo;
            socket.send(sendPacket);
        }
        else
            cout << "Connection non accepted" << endl;

        Clock timer;
        while (done)
        {

            /*Packet receivePacket;
            string username, message;

            socket.receive(receivePacket);
            receivePacket >> username >> message;*/

            //cout << "Message de " << username << ": " << message << "----" << receivePacket.getData() << endl;

            std::string s_tex;
            std::cout << "\nYour text: ";

            std::cin >> s_tex;

            if (s_tex == "exit")
                done = false;

            msgSend = s_tex;

            Packet sendPacket;
            //globalMutex.lock();
            sendPacket << pseudo << msgSend;
            socket.send(sendPacket);
            //globalMutex.unlock();
            wxMilliSleep(2000);
        }

    }

    /*for(int i = 0; i < this->th_iter; i++){
        std::cout<< "Test th_wx-> "<< i<< std::endl;
        wxMilliSleep(1000);
    }*/
    //wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED,
                         //ID_COUNTED);
    //wxGetApp().AddPendingEvent(event);

}