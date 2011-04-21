package billing;

import static billing.DataFields.*;

public class ShortMessageRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        final long pipsPerKiloBytes = (Long)state;

        long result = 0;
        List<Slip> slips = (List<Slip>)input.get(UserField.kSlips);
        for (Slip slip : slips) {
            if (slip.type == SlipType.kInternet) {
                result += slip.data * pipsPerKiloBytes;
            }
        }

        return result;
    }
}

