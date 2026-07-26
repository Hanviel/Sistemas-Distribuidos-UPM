// clase estática que contacta con el Registry para obtener la
// referencia remota al servicio

// DEBE RELLENAR EL MÉTODO init

package pubsubcln;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.RemoteException;
import java.rmi.NotBoundException;
import pubsub.PubSub;

// no se puede instanciar ni derivar
public final class Client {
    private Client(){};
    static public PubSub init(String host, String port) throws RemoteException, NotBoundException {
        Registry registry = LocateRegistry.getRegistry(host, Integer.parseInt(port));
        return (PubSub) registry.lookup("PubSubService");
    }
}
