package bankqueue.customer;

import java.util.Random;

public class CustomerFactory {
    private static final int kMinServiceTime = 30; // half a minute
    private static final int kMaxServiceTime = 5 * 60; // five minutes

    public static Customer newCustomer(int id, Random r) {
        CustomerType type = CustomerFactory.getCustomerType(r);
        int serviceTime = CustomerFactory.getServiceTime(r, type);

        switch (type) {
        case kNormal:
            return new NormalCustomer(id, serviceTime);
        case kFast:
            return new FastCustomer(id, serviceTime);
        case kVip:
            return new VipCustomer(id, serviceTime);
        }
        return null;
    }

    public static CustomerType getCustomerType(Random r) {
        int rand = r.nextInt(10);
        if (rand % 10 == 0)
            return CustomerType.kVip;
        else if (rand < 4)
            return CustomerType.kFast;
        else
            return CustomerType.kNormal;
    }

    private static int getServiceTime(Random r, CustomerType type) {
        int serviceTime;
        if (type == CustomerType.kFast) {
            serviceTime = kMinServiceTime;
        } else {
            serviceTime = kMinServiceTime + r.nextInt(kMaxServiceTime - kMinServiceTime + 1);
        }
        return serviceTime;
    }
}
