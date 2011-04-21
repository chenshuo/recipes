package billing;

import static billing.DataFields.*;

public class NormalUserRule extends Rule {
    public static final long kPipsPerServedDay = 2 * 10000L;
    public static final long kPipsPhoneCallPerMinute = 4000L;
    public static final long kPipsPerShortMessage = 1000L;
    public static final long kPipsPerKiloBytes = 30;

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kVip;
        List<Package> packages = input.getPackages();
        assert packages.isEmpty();

        int dayServed = input.getInteger(UserField.kDaysServed);
        long[] parameters = (long[])state;
        long noChargeMinutes = parameters[0];
        long noChargeShortMessages = parameters[1];
        long noChargeKiloBytes = parameters[2];

        Rule phoneCall = factory.create("package_phone_call",
            [ 0L, noChargeMinutes, kPipsPhoneCallPerMinute ]);
        Rule shortMessage = factory.create("package_short_message",
            [ 0L, noChargeShortMessages, kPipsPerShortMessage ]);
        Rule internet = factory.create("package_internet",
            [ 0L, noChargeKiloBytes, kPipsPerKiloBytes ]);

        long total = phoneCall.getMoneyInPips(input) +
                     shortMessage.getMoneyInPips(input) +
                     internet.getMoneyInPips(input) +
                     dayServed * kPipsPerServedDay;
        return total;
    }
}
