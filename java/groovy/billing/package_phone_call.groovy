package billing;

import static billing.DataFields.*;

public class PackagePhoneCallRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        long[] parameters = (long[])state;
        long packagePips = parameters[0];
        long noChargeMinutes = parameters[1];
        long pipsPhoneCallPerMinute = parameters[2];
        return calc(input, packagePips, noChargeMinutes, pipsPhoneCallPerMinute);
    }

    long calc(UserMonthUsage input, long packagePips, long noChargeMinutes, long pipsPerMinute) {
        Rule phoneCall = factory.create("base_phone_call", pipsPerMinute);
        long fee = phoneCall.getMoneyInPips(input);
        fee -= noChargeMinutes*pipsPerMinute;
        if (fee < 0) {
            fee = 0;
        }
        return fee + packagePips;
    }
}
