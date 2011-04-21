package billing;

import static billing.DataFields.*;

public class PhoneCallRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        final long pipsPerMinute = (Long)state;

        long result = 0;
        List<Slip> slips = (List<Slip>)input.get(UserField.kSlips);
        for (Slip slip : slips) {
            if (slip.type == SlipType.kPhoneCall) {
                result += slip.data * pipsPerMinute;
            }
        }

        return result;
    }
}

