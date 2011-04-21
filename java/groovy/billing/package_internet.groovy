package billing;

import static billing.DataFields.*;

public class PackageInternetRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        long[] parameters = (long[])state;
        long packagePips = parameters[0];
        long noChargeKiloBytes = parameters[1];
        long pipsPerKiloByte = parameters[2];
        return calc(input, packagePips, noChargeKiloBytes, pipsPerKiloByte);
    }

    long calc(UserMonthUsage input, long packagePips, long noChargeKiloBytes, long pipsPerKiloByte) {
        Rule internet = factory.create("base_internet", pipsPerKiloByte);
        long fee = internet.getMoneyInPips(input);
        fee -= noChargeKiloBytes*pipsPerKiloByte;
        if (fee < 0) {
            fee = 0;
        }
        return fee + packagePips;
    }
}
