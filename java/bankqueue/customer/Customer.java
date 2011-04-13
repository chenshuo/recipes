package bankqueue.customer;

import bankqueue.Bank;

public abstract class Customer {

    public final int id;
    public final int serviceTime;
    
    protected Customer(int id, int serviceTime) {
        this.id = id;
        this.serviceTime = serviceTime;
    }

    public String getTypeName() {
        return getClass().getSimpleName();
    }

    public abstract boolean findSpareWindow(Bank bank);
    public abstract void gotoWindow(Bank bank);
}
