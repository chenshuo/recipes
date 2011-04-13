package bankqueue.customer;

import bankqueue.Bank;
import bankqueue.WindowType;

public class FastCustomer extends Customer {

    public FastCustomer(int id, int serviceTime) {
        super(id, serviceTime);
    }

    @Override
    public boolean findSpareWindow(Bank bank) {
        return bank.hasSpareWindow(WindowType.kFast);
    }

    @Override
    public void gotoWindow(Bank bank) {
        assert bank.hasSpareWindow(WindowType.kFast);
        bank.serveAtWindow(this, WindowType.kFast);
    }
}
