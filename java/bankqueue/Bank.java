package bankqueue;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Random;

import bankqueue.customer.Customer;
import bankqueue.customer.CustomerFactory;
import bankqueue.customer.FastCustomer;
import bankqueue.customer.NormalCustomer;
import bankqueue.customer.VipCustomer;
import bankqueue.event.CustomerArriveEvent;
import bankqueue.event.CustomerLeaveEvent;
import bankqueue.event.EventSimulator;

public class Bank {

    private static final int kMinInterval = 0;
    private static final int kMaxInterval = 60;

    private EventSimulator simulator = new EventSimulator();
    private int[] availableWindows = new int[WindowType.kNumWindows.ordinal()];
    private int now = 0;
    private Queue<Customer> waitingQueue = new LinkedList<Customer>();
    private PrintWriter out = new PrintWriter(System.out);

    public static void main(String[] args) throws Exception {
        int numCustomers = 20;
        if (args.length > 0) {
            numCustomers = Integer.parseInt(args[0]);
        }

        Bank bank = new Bank();
        bank.simulate(numCustomers);
    }

    public Bank() {
        availableWindows[WindowType.kNormal.ordinal()] = 4;
        availableWindows[WindowType.kFast.ordinal()] = 1;
        availableWindows[WindowType.kVip.ordinal()] = 1;
    }

    public void simulate(int numCustomers) {
        generateCustomers(numCustomers);
        simulator.run();
    }

    private void generateCustomers(int numCustomers) {
        Random r = new Random();
        int now = 0;
        for (int i = 0; i < numCustomers; ++i) {
            Customer customer = CustomerFactory.newCustomer(i, r);
            now += kMinInterval + r.nextInt(kMaxInterval - kMinInterval + 1);
            simulator.addEvent(new CustomerArriveEvent(now, customer, this));
        }
    }

    public void dryrun(List<CustomerArriveEvent> events, StringWriter sw) {
        out = new PrintWriter(sw);
        for (CustomerArriveEvent ev : events) {
            simulator.addEvent(ev);
        }
        simulator.run();
    }

    public void arrive(int now, Customer customer) {
        out.printf("time %5d %16s(%5d) arrives, service time = %d\n",
                now, customer.getTypeName(), customer.id, customer.serviceTime);
        assert now >= this.now;
        this.now = now;
        if (customer.findSpareWindow(this)) {
            customer.gotoWindow(this);
        } else {
            enqueueCustomer(customer);
        }
    }

    private void enqueueCustomer(Customer customer) {
        waitingQueue.add(customer);
        out.printf("time %5d %16s(%5d) waits in queue\n", now,
                customer.getTypeName(), customer.id);
    }

    public void leave(int now, Customer customer, WindowType type) {
        assert now >= this.now;
        this.now = now;
        out.printf("time %5d %16s(%5d) leaves window %s\n", now,
                customer.getTypeName(), customer.id, type);
        ++availableWindows[type.ordinal()];

        assert hasAnySpareWindow();
        Customer nextCustomer = getNextCustomer(type);

        if (nextCustomer != null) {
            out.printf("time %5d %16s(%5d) found for spare window %s\n",
                    now, nextCustomer.getTypeName(), nextCustomer.id, type);
            assert nextCustomer.findSpareWindow(this);
            nextCustomer.gotoWindow(this);
        } else {
            out.printf("time %5d   %21s spare window %s\n", now, "", type);
        }
    }

    // FIXME: type-switch
    private Customer getNextCustomer(WindowType type) {
        Customer nextCustomer = null;
        switch (type) {
        case kNormal:
            nextCustomer = dequeueFirstCustomer(NormalCustomer.class);
            break;
        case kFast:
            nextCustomer = dequeueFirstCustomer(FastCustomer.class);
            if (nextCustomer == null) {
                nextCustomer = dequeueFirstCustomer(NormalCustomer.class);
            }
            break;
        case kVip:
            nextCustomer = dequeueFirstCustomer(VipCustomer.class);
            if (nextCustomer == null) {
                nextCustomer = dequeueFirstCustomer(NormalCustomer.class);
            }
            break;
        }
        return nextCustomer;
    }

    private <E> Customer dequeueFirstCustomer(Class<E> clazz) {
        Customer toRemove = null;
        for (Customer customer : waitingQueue) {
            if (customer.getClass().equals(clazz)) {
                toRemove = customer;
                break;
            }
        }
        if (toRemove != null) {
            waitingQueue.remove(toRemove);
        }
        return toRemove;
    }

    public boolean hasSpareWindow(WindowType type) {
        return availableWindows[type.ordinal()] > 0;
    }

    public boolean hasAnySpareWindow() {
        return hasSpareWindow(WindowType.kNormal)
                || hasSpareWindow(WindowType.kFast)
                || hasSpareWindow(WindowType.kVip);
    }

    public void serveAtWindow(Customer customer, WindowType type) {
        assert availableWindows[type.ordinal()] > 0;
        --availableWindows[type.ordinal()];
        int leaveAt = now + customer.serviceTime;
        out.printf("time %5d %16s(%5d) sits at %s window, will leave at %d\n",
                now, customer.getTypeName(), customer.id, type, leaveAt);
        simulator.addEvent(new CustomerLeaveEvent(leaveAt, customer, this, type));
    }
}
