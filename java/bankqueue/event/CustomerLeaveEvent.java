package bankqueue.event;

import bankqueue.Bank;
import bankqueue.WindowType;
import bankqueue.customer.Customer;

public class CustomerLeaveEvent extends Event {

    private final Customer customer;
    private final Bank bank;
    private final WindowType type;

    public CustomerLeaveEvent(int time, Customer customer, Bank bank, WindowType type) {
        super(time);
        this.customer = customer;
        this.bank = bank;
        this.type = type;
    }

    @Override
    public void happen(EventSimulator simulator) {
        assert simulator.getNow() == scheduledTime;

        bank.leave(simulator.getNow(), customer, type);
    }
}
