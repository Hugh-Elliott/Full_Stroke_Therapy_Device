
if (exist('out', 'var'))
    outExists = 1;
else
    outExists = 0;
end

if (exist('outSimple', 'var'))
    simpleExists = 1;
else
    simpleExists = 0;
end

if (exist('outCompliant', 'var'))
    compliantExists = 1;
else
    compliantExists = 0;
end

exptemp = size(time);
expSize = exptemp(2);

if (simpleExists)
simpletemp = size(outSimple.Pos.Time);
simpleSize = simpletemp(1);
else
    simpleSize = 2*10^9;
end

if (compliantExists)
complianttemp = size(outCompliant.LinearStateVector.x1.Time);
compliantSize = complianttemp(1);
else
    compliantSize = 2*10^9;
end

if (outExists)
    assisttemp = size(out.Pos.Time);
    assistSize = assisttemp(1);
    minData = min([simpleSize, compliantSize, expSize, assistSize]);
else
    minData = min([simpleSize, compliantSize, expSize]);
end

t_ref = linspace(0, max(time), minData);

pos_expInterp = interp1(time, pos, t_ref, "linear", "extrap");

if(outExists)
    assistPosInterp = interp1(out.Pos.Time, out.Pos.Data, t_ref, "linear", "extrap");
    assistError = assistPosInterp - pos_expInterp;
    assistRMSE = sqrt(mean(assistError.^2))
    assistMax = max(assistError)
    % assistSum = sum(assistError)
    assistMean = mean(abs(assistPosInterp - pos_expInterp))
    assistMeanBias = mean(assistPosInterp - pos_expInterp)
    assistSTD = std(assistPosInterp - pos)
    % pause
end

if (simpleExists)
simplePosInterp = interp1(outSimple.Pos.Time, outSimple.Pos.Data, t_ref, "linear", "extrap");
simpleError = simplePosInterp - pos_expInterp;
simpleRMSE = sqrt(mean(simpleError.^2))
simpleMax = max(simpleError)
% simpleSum = sum(simpleError)
simpleMean = mean(abs(simplePosInterp - pos_expInterp))
simpleMeanBias = mean(simplePosInterp - pos_expInterp)
simpleSTD = std(simplePosInterp - pos_expInterp)
% pause
end

if (compliantExists)
compliantPosInterp = interp1(outCompliant.LinearStateVector.x1.Time, outCompliant.LinearStateVector.x1.Data, t_ref, "linear", "extrap");
compliantError = compliantPosInterp - pos_expInterp;
compliantRMSE = sqrt(mean(compliantError.^2))
compliantMax = max(compliantError)
% compliantSum = sum(compliantError)
compliantMean = mean(abs(compliantPosInterp - pos_expInterp))
compliantMeanBias = mean(compliantPosInterp - pos_expInterp)
compliantSTD = std(compliantPosInterp - pos_expInterp)
end