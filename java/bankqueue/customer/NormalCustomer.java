package bankqueue.customer;

import bankqueue.Bank;
import bankqueue.WindowType;

public class NormalCustomer extends Customer {

    public NormalCustomer(int id, int serviceTime) {
        super(id, serviceTime);
    }

    @Override
    public boolean findSpareWindow(Bank bank) {
        return bank.hasAnySpareWindow();
    }

    @Override
    public void gotoWindow(Bank bank) {
        if (bank.hasSpareWindow(WindowType.kNormal)) {
            bank.serveAtWindow(this, WindowType.kNormal);
        } else if (bank.hasSpareWindow(WindowType.kFast)) {
            bank.serveAtWindow(this, WindowType.kFast);
        } else if (bank.hasSpareWindow(WindowType.kVip)) {
            bank.serveAtWindow(this, WindowType.kVip);
        } else {
            assert false;
        }
    }
}
