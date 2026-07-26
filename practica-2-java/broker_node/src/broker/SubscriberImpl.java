// Clase que implementa la interfaz remota Subscriber
package broker;
import java.rmi.RemoteException;
import java.rmi.NoSuchObjectException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.UnicastRemoteObject;
import java.util.Collection;
import java.util.UUID;
import pubsub.Subscriber;
import pubsub.SubscriberCallback;
import pubsub.Event;
import java.util.Queue;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;
import java.util.ArrayList;
import java.nio.file.PathMatcher;
import java.nio.file.FileSystems;
import java.nio.file.Paths;

class SubscriberImpl extends UnicastRemoteObject implements Subscriber  {
    public static final long serialVersionUID=1234567890L;
    UUID subUUID; // para facilitar depuración
    PubSubImpl ps; // para acceder a funcionalidad del servicio general
    // para notificar al subscriptor de creación y destrucción de temas
    transient SubscriberCallback scbk; 

    Queue<Event> events = new LinkedList<>();
    Set<String> subscribedTopics = new HashSet<>();
    boolean active = true;

    public SubscriberImpl(PubSubImpl p, SubscriberCallback s) throws RemoteException {
        scbk=s;
        subUUID = UUID.randomUUID();
        ps=p;
    }

    private void checkActive() throws RemoteException {
        if (!active) throw new NoSuchObjectException("this subscriber has already finished");
    }

    public UUID getUUID() throws RemoteException {
        checkActive();
        return subUUID;
    }
    public int subscribe(String topic, boolean glob) throws RemoteException {
        checkActive();
        int count = 0;
        if (glob) {
            PathMatcher matcher = FileSystems.getDefault().getPathMatcher("glob:" + topic);
            for (String t : ps.topicList()) {
                if (matcher.matches(Paths.get(t))) {
                    Topic tp = ps.topics.get(t);
                    if (tp != null && !subscribedTopics.contains(t)) {
                        tp.addSubscriber(this);
                        subscribedTopics.add(t);
                        count++;
                    }
                }
            }
        } else {
            Topic tp = ps.topics.get(topic);
            if (tp != null && !subscribedTopics.contains(topic)) {
                tp.addSubscriber(this);
                subscribedTopics.add(topic);
                count++;
            }
        }
        return count;
    }
    public Event getEvent() throws RemoteException {
        checkActive();
        return events.poll();
    }
    public Collection<String> topicListBySubscriber() throws RemoteException {
        checkActive();
        return new ArrayList<>(subscribedTopics);
    }
    public boolean unsubscribe(String topic) throws RemoteException {
        checkActive();
        if (subscribedTopics.remove(topic)) {
            Topic tp = ps.topics.get(topic);
            if (tp != null) tp.removeSubscriber(this);
            return true;
        }
        return false;
    }
    public void exit() throws RemoteException {
        checkActive();
        active = false;
        for (String t : subscribedTopics) {
            Topic tp = ps.topics.get(t);
            if (tp != null) tp.removeSubscriber(this);
        }
        ps.removeSubscriber(this);
        UnicastRemoteObject.unexportObject(this, true);
    }

    public void addEvent(Event e) {
        events.add(e);
    }
}
