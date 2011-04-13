package bankqueue.customer;

import bankqueue.Bank;
import bankqueue.WindowType;

public class VipCustomer extends Customer {

    public VipCustomer(int id, int serviceTime) {
        super(id, serviceTime);
    }

    @Override
    public boolean findSpareWindow(Bank bank) {
        return bank.hasSpareWindow(WindowType.kVip);
    }

    @Override
    public void gotoWindow(Bank bank) {
        assert bank.hasSpareWindow(WindowType.kVip);
        bank.serveAtWindow(this, WindowType.kVip);
    }
}
