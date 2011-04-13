package bankqueue.event;

import java.util.concurrent.atomic.AtomicInteger;

public abstract class Event implements Comparable<Event> {

    private static AtomicInteger numEventsCreated = new AtomicInteger();

    public final int scheduledTime;
    public final int tieBreaker;

    public Event(int time) {
        this.scheduledTime = time;
        this.tieBreaker = numEventsCreated.getAndIncrement();
    }

    public int compareTo(Event rhs) {
        if (scheduledTime != rhs.scheduledTime)
            return scheduledTime - rhs.scheduledTime;
        else
            return tieBreaker - rhs.tieBreaker;
    }

    public abstract void happen(EventSimulator simulator);
}
