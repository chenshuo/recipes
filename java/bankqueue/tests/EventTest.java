package bankqueue.tests;

import static org.junit.Assert.assertEquals;

import java.util.PriorityQueue;

import org.junit.Test;

import bankqueue.event.Event;
import bankqueue.event.EventSimulator;

public class EventTest {

    @Test
    public void testCompareTo1() {
        PriorityQueue<Event> queue = new PriorityQueue<Event>();
        queue.add(new Event(1) {
            @Override
            public void happen(EventSimulator simulator) {
            }
        });
        queue.add(new Event(2) {
            @Override
            public void happen(EventSimulator simulator) {
            }
        });
        assertEquals(1, queue.poll().scheduledTime);
        assertEquals(2, queue.poll().scheduledTime);
    }

    @Test
    public void testCompareTo2() {
        PriorityQueue<Event> queue = new PriorityQueue<Event>();
        queue.add(new Event(2) {
            @Override
            public void happen(EventSimulator simulator) {
            }
        });
        queue.add(new Event(1) {
            @Override
            public void happen(EventSimulator simulator) {
            }
        });
        assertEquals(1, queue.poll().scheduledTime);
        assertEquals(2, queue.poll().scheduledTime);
    }

}
