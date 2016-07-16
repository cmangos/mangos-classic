<?php

class Spawn
{
    const OBJECT_TYPE_GAMEOBJECT    = 'gameobject';
    const OBJECT_TYPE_CREATURE      = 'creature';

    public $guid;
    public $id;
    public $map;
    public $x;
    public $y;
    public $z;
    public $poolId;

    public function __construct(array $row)
    {
        $this->guid = $row['guid'];
        $this->id = $row['id'];
        $this->map = $row['map'];
        $this->x = $row['x'];
        $this->y = $row['y'];
        $this->z = $row['z'];
        $this->poolId = $row['poolId'] != null ? $row['poolId'] : 0;
    }

    public function getOverlappingSpawns(array $spawns)
    {
        $overlappingSpawns = array();

        foreach ($spawns as $spawn)
        {
            if (($this->poolId == 0 || $this->poolId != $spawn->poolId || $spawn->poolId == 0) && $this->guid != $spawn->guid && $this->map == $spawn->map && $this->getDistance3d($spawn) <= 1)
            {
                $overlappingSpawns[] = $spawn;
            }
        }

        return $overlappingSpawns;
    }

    public function getDistance3d(Spawn $spawn)
    {
        return sqrt(pow($this->x - $spawn->x, 2) + pow($this->y - $spawn->y, 2) + pow($this->z - $spawn->z, 2));
    }

    public function getDistance2d(Spawn $spawn)
    {
        return sqrt(pow($this->x - $spawn->x, 2) + pow($this->y - $spawn->y, 2));
    }

    public static function loadAll($objectType)
    {
        $spawns = array();
        $db = new PDO("mysql:host=localhost;dbname=mangos", "root", "");
        $result = $db->query("SELECT s.guid as guid, s.id as id, s.map as map, s.position_x as x, s.position_y as y, s.position_z as z, p.pool_entry as poolId FROM $objectType s LEFT JOIN pool_$objectType p ON s.guid = p.guid");
        while ($row = $result->fetch())
            $spawns[] = new Spawn($row);
        return $spawns;
    }
}

$spawns = Spawn::loadAll(Spawn::OBJECT_TYPE_GAMEOBJECT);
$spawnsPrinted = array();
foreach ($spawns as $key => $spawn)
{
    $overlappingSpawns = $spawn->getOverlappingSpawns($spawns);
    if (!isset($spawnsPrinted[$spawn->guid]) && count($overlappingSpawns) > 0)
    {
        $spawnsPrinted[$spawn->guid] = true;
        echo "Gameobject (GUID: $spawn->guid | ID: $spawn->id | MAP: $spawn->map | POOL: $spawn->poolId) has overlapping spawns\n";
        foreach ($overlappingSpawns as $ospawn)
        {
            $spawnsPrinted[$ospawn->guid] = true;
            echo "====> Gameobject (GUID: $ospawn->guid | ID: $ospawn->id | MAP: $ospawn->map | POOL: $ospawn->poolId) Distance3D: " . $ospawn->getDistance3d($spawn) . " | Distance2D: " . $ospawn->getDistance2d($spawn) . " \n";
        }
        echo "\n\n";
    }
}
