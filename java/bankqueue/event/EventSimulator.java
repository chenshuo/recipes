package bankqueue.event;

import java.util.PriorityQueue;

public class EventSimulator {

    private int now = 0;
    private PriorityQueue<Event> queue = new PriorityQueue<Event>();

    public int getNow() {
        return now;
    }

    public void addEvent(Event e) {
        queue.add(e);
    }

    public void run() {
        Event ev = queue.poll();
        while (ev != null) {
            assert ev.scheduledTime >= now;
            now = ev.scheduledTime;
            ev.happen(this);
            ev = queue.poll();
        }
    }
}
