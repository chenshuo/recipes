package billing;

import static billing.DataFields.*;

public class ShortMessageRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        final long pipsPerMessage = (Long)state;

        long result = 0;
        List<Slip> slips = (List<Slip>)input.get(UserField.kSlips);
        for (Slip slip : slips) {
            if (slip.type == SlipType.kShortMessage) {
                result += slip.data * pipsPerMessage;
            }
        }

        return result;
    }
}
