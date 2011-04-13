package bankqueue.event;

import bankqueue.Bank;
import bankqueue.customer.Customer;

public class CustomerArriveEvent extends Event {

    private final Customer customer;
    private final Bank bank;

    public CustomerArriveEvent(int time, Customer customer, Bank bank) {
        super(time);
        this.customer = customer;
        this.bank = bank;
    }

    @Override
    public void happen(EventSimulator simulator) {
        assert simulator.getNow() == scheduledTime;

        bank.arrive(simulator.getNow(), customer);
    }
}
