// clase estática que contacta con el Registry para dar de alta el servicio
package broker;
import java.rmi.RemoteException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import pubsub.PubSub;

public final class Server {
    private Server(){};
    static void init(PubSub pubsub, String port) throws RemoteException {
        Registry registry = LocateRegistry.getRegistry(Integer.parseInt(port));
        registry.rebind("PubSubService", pubsub);
    }
}
