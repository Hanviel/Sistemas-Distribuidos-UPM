// clase Topic
package broker;
import pubsub.Event;
import pubsub.Subscriber;
import java.util.Queue;
import java.util.LinkedList;
import java.util.List;
import java.util.ArrayList;

class Topic {
    Queue<Event> events = new LinkedList<>();
    List<SubscriberImpl> subscribers = new ArrayList<>();

    public Topic() {
    }

    public void addEvent(Event e) {
        events.add(e);
    }

    public Event getEvent() {
        return events.poll();
    }

    public void addSubscriber(SubscriberImpl s) {
        if (!subscribers.contains(s)) subscribers.add(s);
    }

    public void removeSubscriber(SubscriberImpl s) {
        subscribers.remove(s);
    }

    public List<SubscriberImpl> getSubscribers() {
        return subscribers;
    }
}
