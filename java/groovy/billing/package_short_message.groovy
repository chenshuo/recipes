package billing;

import static billing.DataFields.*;

public class PackageShortMessageRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        long[] parameters = (long[])state;
        long packagePips = parameters[0];
        long noChargeMessages = parameters[1];
        long pipsPerMessage = parameters[2];
        return calc(input, packagePips, noChargeMessages, pipsPerMessage);
    }

    long calc(UserMonthUsage input, long packagePips, long noChargeMessages, long pipsPerMessage) {
        Rule messageRule = factory.create("base_short_message", pipsPerMessage);
        long fee = messageRule.getMoneyInPips(input);
        fee -= noChargeMessages*pipsPerMessage;
        if (fee < 0) {
            fee = 0;
        }
        return fee + packagePips;
    }
}
