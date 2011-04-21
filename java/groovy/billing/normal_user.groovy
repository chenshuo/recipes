package billing;

import static billing.DataFields.*;

public class NormalUserRule extends Rule {

    @Override
    long getMoneyInPips(UserMonthUsage input) {
        UserType type = (UserType)input.get(UserField.kUserType);
        assert type == UserType.kNormal;
        boolean isNew = input.getBoolean(UserField.kIsNewUser);
        if (isNew) {
            Rule newUser = factory.create(type.getRuleName()+"_newjoiner");
            return newUser.getMoneyInPips(input);
        } else {
            Rule phoneCall = factory.create(type.getRuleName()+"_phone_call", 0L);
            Rule shortMessage = factory.create(type.getRuleName()+"_short_message", 0L);
            Rule internet = factory.create(type.getRuleName()+"_internet", 0L);
            long total = phoneCall.getMoneyInPips(input) +
                         shortMessage.getMoneyInPips(input) +
                         internet.getMoneyInPips(input);
            return total;
        }
    }
}
