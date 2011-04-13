package bankqueue.tests;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Random;
import static org.junit.Assert.assertEquals;
import org.junit.Before;
import org.junit.Test;

import bankqueue.Bank;
import bankqueue.customer.CustomerFactory;
import bankqueue.customer.CustomerType;
import bankqueue.customer.FastCustomer;
import bankqueue.customer.NormalCustomer;
import bankqueue.customer.VipCustomer;
import bankqueue.event.CustomerArriveEvent;

public class BankTest {
    ArrayList<CustomerArriveEvent> events;
    StringWriter sw;
    Bank bank;

    String concat(String... lines) {
        StringBuilder sb = new StringBuilder();
        for (String line : lines) {
            sb.append(line).append("\n");
        }
        return sb.toString();
    }

    @Before
    public void setUp() {
        events = new ArrayList<CustomerArriveEvent>();
        sw = new StringWriter();
        bank = new Bank();
    }

    @Test
    public void testGetCustomerType() {
        Random r = new Random();
        final int numTotolCustomers = 100000;

        int numNormalCustomers = 0;
        int numFastCustomers = 0;
        int numVipCustomers = 0;
        for (int i = 0; i < numTotolCustomers; ++i) {
            CustomerType type = CustomerFactory.getCustomerType(r);

            switch (type) {
            case kNormal:
                ++numNormalCustomers;
                break;
            case kFast:
                ++numFastCustomers;
                break;
            case kVip:
                ++numVipCustomers;
                break;
            }
        }

        double delta = 0.01;
        assertEquals(0.6, 1.0 * numNormalCustomers / numTotolCustomers, delta);
        assertEquals(0.3, 1.0 * numFastCustomers / numTotolCustomers, delta);
        assertEquals(0.1, 1.0 * numVipCustomers / numTotolCustomers, delta);
    }

    @Test
    public void noCustomer() {
        bank.dryrun(events, sw);
        assertEquals("", sw.toString());
    }

    @Test
    public void oneNormalCustomer() {
        events.add(new CustomerArriveEvent(0, new NormalCustomer(0, 5), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0   NormalCustomer(    0) arrives, service time = 5",
                "time     0   NormalCustomer(    0) sits at kNormal window, will leave at 5",
                "time     5   NormalCustomer(    0) leaves window kNormal",
                "time     5                         spare window kNormal"),
                sw.toString());
    }

    @Test
    public void oneFastCustomer() {
        events.add(new CustomerArriveEvent(0, new FastCustomer(0, 5), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0     FastCustomer(    0) arrives, service time = 5",
                "time     0     FastCustomer(    0) sits at kFast window, will leave at 5",
                "time     5     FastCustomer(    0) leaves window kFast",
                "time     5                         spare window kFast"),
                sw.toString());
    }

    @Test
    public void oneVipCustomer() {
        events.add(new CustomerArriveEvent(0, new VipCustomer(0, 5), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0      VipCustomer(    0) arrives, service time = 5",
                "time     0      VipCustomer(    0) sits at kVip window, will leave at 5",
                "time     5      VipCustomer(    0) leaves window kVip",
                "time     5                         spare window kVip"),
                sw.toString());
    }

    @Test
    public void twoFastCustomers() {
        events.add(new CustomerArriveEvent(0, new FastCustomer(0, 5), bank));
        events.add(new CustomerArriveEvent(10, new FastCustomer(1, 8), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0     FastCustomer(    0) arrives, service time = 5",
                "time     0     FastCustomer(    0) sits at kFast window, will leave at 5",
                "time     5     FastCustomer(    0) leaves window kFast",
                "time     5                         spare window kFast",
                "time    10     FastCustomer(    1) arrives, service time = 8",
                "time    10     FastCustomer(    1) sits at kFast window, will leave at 18",
                "time    18     FastCustomer(    1) leaves window kFast",
                "time    18                         spare window kFast"
                ),
                sw.toString());
    }

    @Test
    public void twoVipCustomers() {
        events.add(new CustomerArriveEvent(0, new VipCustomer(0, 10), bank));
        events.add(new CustomerArriveEvent(5, new VipCustomer(1, 10), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0      VipCustomer(    0) arrives, service time = 10",
                "time     0      VipCustomer(    0) sits at kVip window, will leave at 10",
                "time     5      VipCustomer(    1) arrives, service time = 10",
                "time     5      VipCustomer(    1) waits in queue",
                "time    10      VipCustomer(    0) leaves window kVip",
                "time    10      VipCustomer(    1) found for spare window kVip",
                "time    10      VipCustomer(    1) sits at kVip window, will leave at 20",
                "time    20      VipCustomer(    1) leaves window kVip",
                "time    20                         spare window kVip"
                ),
                sw.toString());
    }

    @Test
    public void sixNormalCustomers() {
        events.add(new CustomerArriveEvent(0, new NormalCustomer(0, 10), bank));
        events.add(new CustomerArriveEvent(1, new NormalCustomer(1, 10), bank));
        events.add(new CustomerArriveEvent(2, new NormalCustomer(2, 10), bank));
        events.add(new CustomerArriveEvent(3, new NormalCustomer(3, 10), bank));
        events.add(new CustomerArriveEvent(4, new NormalCustomer(4, 10), bank));
        events.add(new CustomerArriveEvent(5, new NormalCustomer(5, 10), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0   NormalCustomer(    0) arrives, service time = 10",
                "time     0   NormalCustomer(    0) sits at kNormal window, will leave at 10",
                "time     1   NormalCustomer(    1) arrives, service time = 10",
                "time     1   NormalCustomer(    1) sits at kNormal window, will leave at 11",
                "time     2   NormalCustomer(    2) arrives, service time = 10",
                "time     2   NormalCustomer(    2) sits at kNormal window, will leave at 12",
                "time     3   NormalCustomer(    3) arrives, service time = 10",
                "time     3   NormalCustomer(    3) sits at kNormal window, will leave at 13",
                "time     4   NormalCustomer(    4) arrives, service time = 10",
                "time     4   NormalCustomer(    4) sits at kFast window, will leave at 14",
                "time     5   NormalCustomer(    5) arrives, service time = 10",
                "time     5   NormalCustomer(    5) sits at kVip window, will leave at 15",
                "time    10   NormalCustomer(    0) leaves window kNormal",
                "time    10                         spare window kNormal",
                "time    11   NormalCustomer(    1) leaves window kNormal",
                "time    11                         spare window kNormal",
                "time    12   NormalCustomer(    2) leaves window kNormal",
                "time    12                         spare window kNormal",
                "time    13   NormalCustomer(    3) leaves window kNormal",
                "time    13                         spare window kNormal",
                "time    14   NormalCustomer(    4) leaves window kFast",
                "time    14                         spare window kFast",
                "time    15   NormalCustomer(    5) leaves window kVip",
                "time    15                         spare window kVip"
                ),
                sw.toString());
    }

    @Test
    public void sevenCustomers() {
        events.add(new CustomerArriveEvent(0, new VipCustomer(0, 10), bank));
        events.add(new CustomerArriveEvent(1, new NormalCustomer(1, 10), bank));
        events.add(new CustomerArriveEvent(2, new NormalCustomer(2, 10), bank));
        events.add(new CustomerArriveEvent(3, new NormalCustomer(3, 10), bank));
        events.add(new CustomerArriveEvent(4, new NormalCustomer(4, 10), bank));
        events.add(new CustomerArriveEvent(5, new NormalCustomer(5, 10), bank));
        events.add(new CustomerArriveEvent(6, new NormalCustomer(6, 10), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0      VipCustomer(    0) arrives, service time = 10",
                "time     0      VipCustomer(    0) sits at kVip window, will leave at 10",
                "time     1   NormalCustomer(    1) arrives, service time = 10",
                "time     1   NormalCustomer(    1) sits at kNormal window, will leave at 11",
                "time     2   NormalCustomer(    2) arrives, service time = 10",
                "time     2   NormalCustomer(    2) sits at kNormal window, will leave at 12",
                "time     3   NormalCustomer(    3) arrives, service time = 10",
                "time     3   NormalCustomer(    3) sits at kNormal window, will leave at 13",
                "time     4   NormalCustomer(    4) arrives, service time = 10",
                "time     4   NormalCustomer(    4) sits at kNormal window, will leave at 14",
                "time     5   NormalCustomer(    5) arrives, service time = 10",
                "time     5   NormalCustomer(    5) sits at kFast window, will leave at 15",
                "time     6   NormalCustomer(    6) arrives, service time = 10",
                "time     6   NormalCustomer(    6) waits in queue",
                "time    10      VipCustomer(    0) leaves window kVip",
                "time    10   NormalCustomer(    6) found for spare window kVip",
                "time    10   NormalCustomer(    6) sits at kVip window, will leave at 20",
                "time    11   NormalCustomer(    1) leaves window kNormal",
                "time    11                         spare window kNormal",
                "time    12   NormalCustomer(    2) leaves window kNormal",
                "time    12                         spare window kNormal",
                "time    13   NormalCustomer(    3) leaves window kNormal",
                "time    13                         spare window kNormal",
                "time    14   NormalCustomer(    4) leaves window kNormal",
                "time    14                         spare window kNormal",
                "time    15   NormalCustomer(    5) leaves window kFast",
                "time    15                         spare window kFast",
                "time    20   NormalCustomer(    6) leaves window kVip",
                "time    20                         spare window kVip"
                ),
                sw.toString());
    }

    @Test
    public void eightCustomers() {
        events.add(new CustomerArriveEvent(0, new VipCustomer(0, 10), bank));
        events.add(new CustomerArriveEvent(1, new NormalCustomer(1, 10), bank));
        events.add(new CustomerArriveEvent(2, new NormalCustomer(2, 10), bank));
        events.add(new CustomerArriveEvent(3, new NormalCustomer(3, 10), bank));
        events.add(new CustomerArriveEvent(4, new NormalCustomer(4, 10), bank));
        events.add(new CustomerArriveEvent(5, new NormalCustomer(5, 10), bank));
        events.add(new CustomerArriveEvent(6, new NormalCustomer(6, 10), bank));
        events.add(new CustomerArriveEvent(7, new VipCustomer(7, 10), bank));
        bank.dryrun(events, sw);
        assertEquals(concat(
                "time     0      VipCustomer(    0) arrives, service time = 10",
                "time     0      VipCustomer(    0) sits at kVip window, will leave at 10",
                "time     1   NormalCustomer(    1) arrives, service time = 10",
                "time     1   NormalCustomer(    1) sits at kNormal window, will leave at 11",
                "time     2   NormalCustomer(    2) arrives, service time = 10",
                "time     2   NormalCustomer(    2) sits at kNormal window, will leave at 12",
                "time     3   NormalCustomer(    3) arrives, service time = 10",
                "time     3   NormalCustomer(    3) sits at kNormal window, will leave at 13",
                "time     4   NormalCustomer(    4) arrives, service time = 10",
                "time     4   NormalCustomer(    4) sits at kNormal window, will leave at 14",
                "time     5   NormalCustomer(    5) arrives, service time = 10",
                "time     5   NormalCustomer(    5) sits at kFast window, will leave at 15",
                "time     6   NormalCustomer(    6) arrives, service time = 10",
                "time     6   NormalCustomer(    6) waits in queue",
                "time     7      VipCustomer(    7) arrives, service time = 10",
                "time     7      VipCustomer(    7) waits in queue",
                "time    10      VipCustomer(    0) leaves window kVip",
                "time    10      VipCustomer(    7) found for spare window kVip",
                "time    10      VipCustomer(    7) sits at kVip window, will leave at 20",
                "time    11   NormalCustomer(    1) leaves window kNormal",
                "time    11   NormalCustomer(    6) found for spare window kNormal",
                "time    11   NormalCustomer(    6) sits at kNormal window, will leave at 21",
                "time    12   NormalCustomer(    2) leaves window kNormal",
                "time    12                         spare window kNormal",
                "time    13   NormalCustomer(    3) leaves window kNormal",
                "time    13                         spare window kNormal",
                "time    14   NormalCustomer(    4) leaves window kNormal",
                "time    14                         spare window kNormal",
                "time    15   NormalCustomer(    5) leaves window kFast",
                "time    15                         spare window kFast",
                "time    20      VipCustomer(    7) leaves window kVip",
                "time    20                         spare window kVip",
                "time    21   NormalCustomer(    6) leaves window kNormal",
                "time    21                         spare window kNormal"
                ),
                sw.toString());
    }
}
